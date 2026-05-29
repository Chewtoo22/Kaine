from __future__ import annotations

import argparse
import html
import json
import mimetypes
import os
import platform
import re
import socket
import sys
import threading
import time
from dataclasses import dataclass
from datetime import datetime, timezone
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from typing import Any
from urllib.parse import unquote, urlparse
from uuid import uuid4


APP_NAME = "Kaine"
APP_VERSION = "0.1.0"
MAX_BODY_BYTES = 64 * 1024
MAX_MESSAGE_CHARS = 4000


def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="seconds")


def local_now_label() -> str:
    return datetime.now().strftime("%A, %B %d, %Y at %I:%M %p")


def slug_id(prefix: str) -> str:
    return f"{prefix}_{uuid4().hex[:10]}"


class KaineStore:
    def __init__(self, data_dir: Path):
        self.data_dir = data_dir
        self.path = data_dir / "kaine_state.json"
        self._lock = threading.Lock()
        self.data_dir.mkdir(parents=True, exist_ok=True)
        self._state = self._load()

    def _default_state(self) -> dict[str, Any]:
        now = utc_now()
        return {
            "profile": {
                "name": APP_NAME,
                "version": APP_VERSION,
                "created_at": now,
                "stance": "local-first operator",
            },
            "conversation": [
                {
                    "id": slug_id("msg"),
                    "role": "assistant",
                    "mode": "Operator",
                    "content": "Kaine online. Local core active. Awaiting directive.",
                    "created_at": now,
                }
            ],
            "memories": [],
            "missions": [
                {
                    "id": slug_id("mission"),
                    "title": "Establish Kaine core identity",
                    "status": "active",
                    "priority": "high",
                    "source": "system",
                    "created_at": now,
                },
                {
                    "id": slug_id("mission"),
                    "title": "Keep first launch useful without external API keys",
                    "status": "active",
                    "priority": "high",
                    "source": "system",
                    "created_at": now,
                },
            ],
        }

    def _load(self) -> dict[str, Any]:
        if not self.path.exists():
            state = self._default_state()
            self._write_state(state)
            return state

        try:
            loaded = json.loads(self.path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            loaded = self._default_state()
            loaded["memories"].append(
                {
                    "id": slug_id("mem"),
                    "kind": "system",
                    "title": "Recovered from invalid state file",
                    "body": "Kaine created a clean runtime state because the previous JSON file could not be parsed.",
                    "created_at": utc_now(),
                }
            )
            self._write_state(loaded)
        return self._normalize(loaded)

    def _normalize(self, state: dict[str, Any]) -> dict[str, Any]:
        state.setdefault("profile", {"name": APP_NAME, "version": APP_VERSION})
        state.setdefault("conversation", [])
        state.setdefault("memories", [])
        state.setdefault("missions", [])
        state["profile"].setdefault("name", APP_NAME)
        state["profile"].setdefault("version", APP_VERSION)
        return state

    def _write_state(self, state: dict[str, Any]) -> None:
        self.data_dir.mkdir(parents=True, exist_ok=True)
        tmp_path = self.path.with_suffix(".tmp")
        tmp_path.write_text(json.dumps(state, indent=2), encoding="utf-8")
        os.replace(tmp_path, self.path)

    def snapshot(self) -> dict[str, Any]:
        with self._lock:
            return json.loads(json.dumps(self._state))

    def save_message(self, role: str, content: str, mode: str = "Operator") -> dict[str, Any]:
        entry = {
            "id": slug_id("msg"),
            "role": role,
            "mode": mode,
            "content": content,
            "created_at": utc_now(),
        }
        with self._lock:
            self._state["conversation"].append(entry)
            self._state["conversation"] = self._state["conversation"][-120:]
            self._write_state(self._state)
        return entry

    def save_memory(self, title: str, body: str, kind: str = "user") -> dict[str, Any]:
        entry = {
            "id": slug_id("mem"),
            "kind": clean_text(kind, 40) or "user",
            "title": clean_text(title, 120) or "Memory",
            "body": clean_text(body, 1200),
            "created_at": utc_now(),
        }
        with self._lock:
            self._state["memories"].insert(0, entry)
            self._state["memories"] = self._state["memories"][:80]
            self._write_state(self._state)
        return entry

    def save_mission(
        self,
        title: str,
        status: str = "active",
        priority: str = "normal",
        source: str = "user",
    ) -> dict[str, Any]:
        entry = {
            "id": slug_id("mission"),
            "title": clean_text(title, 160) or "Untitled mission",
            "status": clean_text(status, 40) or "active",
            "priority": clean_text(priority, 40) or "normal",
            "source": clean_text(source, 80) or "user",
            "created_at": utc_now(),
        }
        with self._lock:
            self._state["missions"].insert(0, entry)
            self._state["missions"] = self._state["missions"][:60]
            self._write_state(self._state)
        return entry


def clean_text(value: Any, max_chars: int) -> str:
    if value is None:
        return ""
    text = str(value).replace("\x00", "").strip()
    text = re.sub(r"[ \t]+", " ", text)
    return text[:max_chars]


@dataclass(frozen=True)
class RuntimeContext:
    root: Path
    web_root: Path
    started_at: float
    host: str
    port: int


class KaineBrain:
    MODES = {"Operator", "Engineer", "Security", "Strategy", "Creative"}

    def __init__(self, root: Path, store: KaineStore, started_at: float):
        self.root = root
        self.store = store
        self.started_at = started_at

    def respond(self, message: str, requested_mode: str) -> dict[str, Any]:
        mode = requested_mode if requested_mode in self.MODES else "Operator"
        text = clean_text(message, MAX_MESSAGE_CHARS)
        lower = text.lower()
        created_memory = self._maybe_capture_memory(text)
        created_mission = self._maybe_create_mission(text)
        intent = self._classify(lower)

        if intent == "identity":
            reply = self._identity_reply()
        elif intent == "status":
            reply = self._status_reply()
        elif intent == "time":
            reply = f"Local time is {local_now_label()}. Runtime is stable and the local core is active."
        elif intent == "security":
            reply = self._security_reply(text)
        elif intent == "build":
            reply = self._build_reply(text, mode)
        elif intent == "memory":
            reply = self._memory_reply(created_memory)
        elif intent == "help":
            reply = self._help_reply()
        else:
            reply = self._mode_reply(text, mode)

        if created_memory:
            reply += f"\n\nMemory captured: {created_memory['title']}."
        if created_mission:
            reply += f"\n\nMission added: {created_mission['title']}."

        return {
            "reply": reply,
            "mode": mode,
            "intent": intent,
            "confidence": self._confidence(intent, text),
            "created_memory": created_memory,
            "created_mission": created_mission,
            "suggested_actions": self._suggested_actions(intent),
        }

    def _classify(self, lower: str) -> str:
        if not lower or lower in {"hi", "hello", "hey"}:
            return "identity"
        if any(term in lower for term in ("who are you", "identity", "kaine", "jarvis")):
            return "identity"
        if any(term in lower for term in ("status", "diagnostic", "health", "runtime", "system")):
            return "status"
        if any(term in lower for term in ("time", "date", "today")):
            return "time"
        if any(term in lower for term in ("secure", "security", "threat", "risk", "permission", "guardrail")):
            return "security"
        if any(term in lower for term in ("build", "implement", "create", "ship", "code", "app", "project")):
            return "build"
        if any(term in lower for term in ("remember", "memory", "note that", "save this")):
            return "memory"
        if any(term in lower for term in ("help", "commands", "what can you do")):
            return "help"
        return "general"

    def _identity_reply(self) -> str:
        return (
            "I am Kaine: an original local-first operator built for planning, engineering, memory, "
            "security review, and controlled action. The current core runs without cloud credentials, "
            "keeps a durable local state file, and exposes only safe read-oriented actions until an explicit "
            "tool permission layer is added."
        )

    def _status_reply(self) -> str:
        uptime = int(time.time() - self.started_at)
        files = safe_workspace_summary(self.root)
        state = self.store.snapshot()
        return (
            f"Kaine core: online. Version {APP_VERSION}. Uptime {uptime}s. "
            f"Workspace items visible: {files['count']}. "
            f"Memories: {len(state.get('memories', []))}. "
            f"Missions: {len(state.get('missions', []))}. "
            "Tool policy: safe actions only; arbitrary command execution is locked."
        )

    def _security_reply(self, text: str) -> str:
        return (
            "Security posture: keep Kaine local-first, require explicit approval before external calls, "
            "redact secrets from logs, and gate every tool behind an allowlist. For this request, I would start "
            "with three controls: scoped workspace access, dry-run previews for file changes, and an audit trail "
            "for every action that touches disk, network, or credentials."
        )

    def _build_reply(self, text: str, mode: str) -> str:
        target = text.strip() or "the requested system"
        if len(target) > 140:
            target = target[:137] + "..."
        if mode == "Engineer":
            opening = "Engineering route:"
        elif mode == "Strategy":
            opening = "Strategic route:"
        else:
            opening = "I will treat that as an executable mission:"
        return (
            f"{opening}\n"
            f"1. Define the exact outcome for {target}.\n"
            "2. Split it into identity, interface, core logic, persistence, and verification.\n"
            "3. Build the smallest impressive slice first, then harden the tool layer.\n"
            "4. Verify startup and one real user workflow before calling it complete.\n"
            "Current capability note: I can plan and remember locally now; direct system automation should be added "
            "behind permissioned tools, not bolted directly onto chat."
        )

    def _memory_reply(self, created_memory: dict[str, Any] | None) -> str:
        if created_memory:
            return "Memory accepted. I will use it as local context in future Kaine sessions."
        memories = self.store.snapshot().get("memories", [])
        if not memories:
            return "No user memories stored yet. Say 'remember that ...' and I will save it locally."
        latest = memories[0]
        return f"Latest memory: {latest['title']} - {latest['body']}"

    def _help_reply(self) -> str:
        return (
            "Available directives: ask for status, switch modes, create a mission, say 'remember that ...', "
            "request an implementation plan, or ask for a security pass. Voice output and browser speech input "
            "are controlled from the interface."
        )

    def _mode_reply(self, text: str, mode: str) -> str:
        memory_count = len(self.store.snapshot().get("memories", []))
        if mode == "Engineer":
            return (
                "Engineer mode: I would turn this into a bounded task, identify the files or systems involved, "
                "make the narrowest useful change, then verify with a real runtime check. "
                f"Local memory entries available: {memory_count}."
            )
        if mode == "Security":
            return self._security_reply(text)
        if mode == "Strategy":
            return (
                "Strategy mode: prioritize the leverage point first. Define what Kaine should decide, what it may "
                "only recommend, and what it must never do without confirmation. Then build the workflow around "
                "those boundaries."
            )
        if mode == "Creative":
            return (
                "Creative mode: Kaine should look precise, not decorative. Strong identity cues are the live core, "
                "tight typography, voice-state motion, and calm contrast between command surfaces and telemetry."
            )
        return (
            "Operator mode: understood. I can convert this into a mission, remember durable context, or produce a "
            "concrete plan. Give me the target and the constraint, and I will reduce it to the next action."
        )

    def _maybe_capture_memory(self, text: str) -> dict[str, Any] | None:
        patterns = [
            r"\bremember that\b(?P<body>.+)",
            r"\bremember\b(?P<body>.+)",
            r"\bnote that\b(?P<body>.+)",
            r"\bsave this\b(?P<body>.+)",
        ]
        for pattern in patterns:
            match = re.search(pattern, text, flags=re.IGNORECASE | re.DOTALL)
            if match:
                body = clean_text(match.group("body"), 1000).strip(" .:-")
                if body:
                    title = body.split(".")[0][:90] or "User note"
                    return self.store.save_memory(title=title, body=body, kind="user")
        return None

    def _maybe_create_mission(self, text: str) -> dict[str, Any] | None:
        stripped = text.strip()
        lower = stripped.lower()
        if lower.startswith("mission:"):
            title = stripped.split(":", 1)[1].strip()
            return self.store.save_mission(title, priority="high", source="chat")
        if any(term in lower for term in ("build ", "implement ", "create ")):
            title = re.sub(r"^(please\s+)?(build|implement|create)\s+", "", stripped, flags=re.IGNORECASE).strip()
            if title:
                return self.store.save_mission(title[:150], priority="normal", source="chat")
        return None

    def _suggested_actions(self, intent: str) -> list[dict[str, str]]:
        if intent == "status":
            return [{"label": "Project snapshot", "prompt": "status and project snapshot"}]
        if intent == "security":
            return [{"label": "Create guardrail mission", "prompt": "mission: design Kaine tool permission guardrails"}]
        if intent == "build":
            return [{"label": "Save architecture mission", "prompt": "mission: expand Kaine into a permissioned desktop assistant"}]
        return [
            {"label": "Status", "prompt": "status"},
            {"label": "Security pass", "prompt": "security posture"},
        ]

    def _confidence(self, intent: str, text: str) -> float:
        if not text:
            return 0.72
        return {
            "identity": 0.86,
            "status": 0.91,
            "time": 0.95,
            "security": 0.84,
            "build": 0.8,
            "memory": 0.82,
            "help": 0.9,
            "general": 0.68,
        }.get(intent, 0.65)


def safe_workspace_summary(root: Path) -> dict[str, Any]:
    try:
        entries = []
        for child in sorted(root.iterdir(), key=lambda p: p.name.lower()):
            if child.name in {".git", "data", "__pycache__"}:
                continue
            entries.append(
                {
                    "name": child.name,
                    "type": "dir" if child.is_dir() else "file",
                    "size": child.stat().st_size if child.is_file() else None,
                }
            )
            if len(entries) >= 24:
                break
        return {"count": len(entries), "entries": entries}
    except OSError:
        return {"count": 0, "entries": []}


def build_status(context: RuntimeContext, store: KaineStore) -> dict[str, Any]:
    state = store.snapshot()
    return {
        "name": APP_NAME,
        "version": APP_VERSION,
        "online": True,
        "host": context.host,
        "port": context.port,
        "url": f"http://{context.host}:{context.port}",
        "uptime_seconds": int(time.time() - context.started_at),
        "local_time": local_now_label(),
        "python": sys.version.split()[0],
        "platform": platform.platform(),
        "hostname": socket.gethostname(),
        "workspace": str(context.root),
        "workspace_summary": safe_workspace_summary(context.root),
        "memory_count": len(state.get("memories", [])),
        "mission_count": len(state.get("missions", [])),
        "tool_policy": "safe-actions-only",
    }


def state_payload(context: RuntimeContext, store: KaineStore) -> dict[str, Any]:
    state = store.snapshot()
    return {
        "status": build_status(context, store),
        "profile": state.get("profile", {}),
        "conversation": state.get("conversation", [])[-40:],
        "memories": state.get("memories", [])[:20],
        "missions": state.get("missions", [])[:20],
    }


def make_handler(context: RuntimeContext, store: KaineStore, brain: KaineBrain):
    class KaineRequestHandler(BaseHTTPRequestHandler):
        server_version = f"{APP_NAME}/{APP_VERSION}"

        def log_message(self, fmt: str, *args: Any) -> None:
            sys.stdout.write(f"[{datetime.now().strftime('%H:%M:%S')}] {self.address_string()} {fmt % args}\n")

        def do_GET(self) -> None:
            parsed = urlparse(self.path)
            if parsed.path == "/api/status":
                self._json(build_status(context, store))
                return
            if parsed.path == "/api/state":
                self._json(state_payload(context, store))
                return
            self._serve_static(parsed.path)

        def do_POST(self) -> None:
            parsed = urlparse(self.path)
            if parsed.path == "/api/chat":
                self._handle_chat()
                return
            if parsed.path == "/api/memory":
                self._handle_memory()
                return
            if parsed.path == "/api/mission":
                self._handle_mission()
                return
            self._json({"error": "unknown endpoint"}, HTTPStatus.NOT_FOUND)

        def _handle_chat(self) -> None:
            payload = self._read_json()
            message = clean_text(payload.get("message"), MAX_MESSAGE_CHARS)
            mode = clean_text(payload.get("mode"), 40) or "Operator"
            if not message:
                self._json({"error": "message is required"}, HTTPStatus.BAD_REQUEST)
                return
            user_message = store.save_message("user", message, mode)
            result = brain.respond(message, mode)
            assistant_message = store.save_message("assistant", result["reply"], result["mode"])
            self._json(
                {
                    "user_message": user_message,
                    "assistant_message": assistant_message,
                    "result": result,
                    "state": state_payload(context, store),
                }
            )

        def _handle_memory(self) -> None:
            payload = self._read_json()
            body = clean_text(payload.get("body"), 1200)
            title = clean_text(payload.get("title"), 120) or (body[:90] if body else "")
            kind = clean_text(payload.get("kind"), 40) or "user"
            if not body:
                self._json({"error": "body is required"}, HTTPStatus.BAD_REQUEST)
                return
            memory = store.save_memory(title=title, body=body, kind=kind)
            self._json({"memory": memory, "state": state_payload(context, store)}, HTTPStatus.CREATED)

        def _handle_mission(self) -> None:
            payload = self._read_json()
            title = clean_text(payload.get("title"), 160)
            if not title:
                self._json({"error": "title is required"}, HTTPStatus.BAD_REQUEST)
                return
            mission = store.save_mission(
                title=title,
                status=clean_text(payload.get("status"), 40) or "active",
                priority=clean_text(payload.get("priority"), 40) or "normal",
                source=clean_text(payload.get("source"), 80) or "manual",
            )
            self._json({"mission": mission, "state": state_payload(context, store)}, HTTPStatus.CREATED)

        def _read_json(self) -> dict[str, Any]:
            try:
                length = int(self.headers.get("Content-Length", "0"))
            except ValueError:
                length = 0
            if length <= 0:
                return {}
            if length > MAX_BODY_BYTES:
                self._json({"error": "request body too large"}, HTTPStatus.REQUEST_ENTITY_TOO_LARGE)
                return {}
            raw = self.rfile.read(length)
            try:
                parsed = json.loads(raw.decode("utf-8"))
                return parsed if isinstance(parsed, dict) else {}
            except json.JSONDecodeError:
                return {}

        def _serve_static(self, request_path: str) -> None:
            relative = unquote(request_path).lstrip("/")
            if not relative:
                relative = "index.html"
            candidate = (context.web_root / relative).resolve()
            try:
                candidate.relative_to(context.web_root)
            except ValueError:
                self._json({"error": "invalid path"}, HTTPStatus.FORBIDDEN)
                return
            if not candidate.exists() or not candidate.is_file():
                self._json({"error": "not found"}, HTTPStatus.NOT_FOUND)
                return
            content_type = mimetypes.guess_type(str(candidate))[0] or "application/octet-stream"
            data = candidate.read_bytes()
            self.send_response(HTTPStatus.OK)
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(data)))
            self.send_header("Cache-Control", "no-store")
            self.end_headers()
            self.wfile.write(data)

        def _json(self, payload: dict[str, Any], status: HTTPStatus = HTTPStatus.OK) -> None:
            data = json.dumps(payload).encode("utf-8")
            self.send_response(status)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Content-Length", str(len(data)))
            self.send_header("Cache-Control", "no-store")
            self.end_headers()
            self.wfile.write(data)

    return KaineRequestHandler


def create_server(
    root: Path | None = None,
    web_root: Path | None = None,
    data_dir: Path | None = None,
    host: str = "127.0.0.1",
    port: int = 8766,
) -> ThreadingHTTPServer:
    resolved_root = (root or Path.cwd()).resolve()
    resolved_web = (web_root or resolved_root / "web").resolve()
    resolved_data = (data_dir or resolved_root / "data").resolve()
    if not resolved_web.exists():
        raise FileNotFoundError(f"web root not found: {resolved_web}")
    started_at = time.time()
    store = KaineStore(resolved_data)
    context = RuntimeContext(
        root=resolved_root,
        web_root=resolved_web,
        started_at=started_at,
        host=host,
        port=port,
    )
    brain = KaineBrain(resolved_root, store, started_at)
    handler = make_handler(context, store, brain)
    server = ThreadingHTTPServer((host, port), handler)
    server.kaine_store = store  # type: ignore[attr-defined]
    return server


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run the Kaine local assistant server.")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8766)
    parser.add_argument("--data-dir", default="data")
    parser.add_argument("--web-root", default="web")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    root = Path.cwd().resolve()
    server = create_server(
        root=root,
        web_root=(root / args.web_root).resolve(),
        data_dir=(root / args.data_dir).resolve(),
        host=args.host,
        port=args.port,
    )
    print(f"Kaine core online: http://{args.host}:{args.port}")
    print("Press Ctrl+C to stop.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nKaine core shutting down.")
    finally:
        server.server_close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
