const api = {
  async get(path) {
    const response = await fetch(path, { headers: { Accept: "application/json" } });
    if (!response.ok) throw new Error(`GET ${path} failed`);
    return response.json();
  },
  async post(path, body) {
    const response = await fetch(path, {
      method: "POST",
      headers: { "Content-Type": "application/json", Accept: "application/json" },
      body: JSON.stringify(body),
    });
    if (!response.ok) {
      const payload = await response.json().catch(() => ({}));
      throw new Error(payload.error || `POST ${path} failed`);
    }
    return response.json();
  },
};

const state = {
  mode: "Operator",
  online: false,
  speaking: false,
  transcript: [],
  memories: [],
  missions: [],
  status: null,
  recognition: null,
};

const els = {
  core: document.querySelector("#kaineCore"),
  subtitle: document.querySelector("#subtitle"),
  statusDot: document.querySelector("#statusDot"),
  connectionLabel: document.querySelector("#connectionLabel"),
  clockLabel: document.querySelector("#clockLabel"),
  coreMetric: document.querySelector("#coreMetric"),
  memoryMetric: document.querySelector("#memoryMetric"),
  missionMetric: document.querySelector("#missionMetric"),
  policyMetric: document.querySelector("#policyMetric"),
  modeLabel: document.querySelector("#modeLabel"),
  transcript: document.querySelector("#transcript"),
  composer: document.querySelector("#composer"),
  promptInput: document.querySelector("#promptInput"),
  listenButton: document.querySelector("#listenButton"),
  speakToggle: document.querySelector("#speakToggle"),
  missionButton: document.querySelector("#missionButton"),
  memoryButton: document.querySelector("#memoryButton"),
  missionList: document.querySelector("#missionList"),
  memoryList: document.querySelector("#memoryList"),
  diagnostics: document.querySelector("#diagnostics"),
  field: document.querySelector("#field"),
};

function escapeHtml(value) {
  return String(value)
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;")
    .replaceAll("'", "&#039;");
}

function setOnline(online) {
  state.online = online;
  els.statusDot.classList.toggle("online", online);
  els.statusDot.classList.toggle("offline", !online);
  els.connectionLabel.textContent = online ? "Local core online" : "Offline fallback";
  els.subtitle.textContent = online ? "Local operator core active" : "Browser-only fallback active";
  els.coreMetric.textContent = online ? "Online" : "Fallback";
}

function renderAll() {
  renderTranscript();
  renderMissions();
  renderMemories();
  renderDiagnostics();
  els.modeLabel.textContent = `${state.mode} mode`;
  els.memoryMetric.textContent = String(state.memories.length);
  els.missionMetric.textContent = String(state.missions.length);
  els.policyMetric.textContent = state.status?.tool_policy || "safe-actions-only";
}

function renderTranscript() {
  els.transcript.innerHTML = state.transcript
    .map((message) => {
      const speaker = message.role === "user" ? "You" : "Kaine";
      return `
        <article class="message ${message.role}">
          <div class="speaker">${speaker}</div>
          <div class="bubble">${escapeHtml(message.content)}</div>
        </article>
      `;
    })
    .join("");
  els.transcript.scrollTop = els.transcript.scrollHeight;
}

function renderMissions() {
  if (!state.missions.length) {
    els.missionList.innerHTML = `<div class="item-card"><strong>No missions</strong><p>Standby.</p></div>`;
    return;
  }
  els.missionList.innerHTML = state.missions
    .map(
      (mission) => `
        <article class="item-card">
          <strong>${escapeHtml(mission.title)}</strong>
          <span>${escapeHtml(mission.status)} / ${escapeHtml(mission.priority)}</span>
          <div class="tag">${escapeHtml(mission.source || "local")}</div>
        </article>
      `,
    )
    .join("");
}

function renderMemories() {
  if (!state.memories.length) {
    els.memoryList.innerHTML = `<div class="item-card"><strong>No memory</strong><p>Awaiting explicit note.</p></div>`;
    return;
  }
  els.memoryList.innerHTML = state.memories
    .map(
      (memory) => `
        <article class="item-card">
          <strong>${escapeHtml(memory.title)}</strong>
          <p>${escapeHtml(memory.body)}</p>
          <div class="tag">${escapeHtml(memory.kind || "user")}</div>
        </article>
      `,
    )
    .join("");
}

function renderDiagnostics() {
  const status = state.status;
  if (!status) {
    els.diagnostics.innerHTML = "<p>Diagnostics unavailable.</p>";
    return;
  }
  const items = [
    ["Version", status.version],
    ["Uptime", `${status.uptime_seconds}s`],
    ["Python", status.python],
    ["Host", `${status.host}:${status.port}`],
    ["Workspace", status.workspace],
  ];
  els.diagnostics.innerHTML = `
    <dl>
      ${items
        .map(([key, value]) => `<dt>${escapeHtml(key)}</dt><dd>${escapeHtml(value || "--")}</dd>`)
        .join("")}
    </dl>
  `;
}

function applyServerState(payload) {
  state.status = payload.status;
  state.transcript = payload.conversation || [];
  state.memories = payload.memories || [];
  state.missions = payload.missions || [];
  setOnline(true);
  renderAll();
}

async function loadState() {
  try {
    const payload = await api.get("/api/state");
    applyServerState(payload);
  } catch (error) {
    setOnline(false);
    loadFallbackState();
    renderAll();
  }
}

function loadFallbackState() {
  const saved = JSON.parse(localStorage.getItem("kaine-fallback") || "{}");
  state.transcript =
    saved.transcript || [
      {
        role: "assistant",
        content: "Kaine browser core active. Start the Python server for persistent memory and diagnostics.",
      },
    ];
  state.memories = saved.memories || [];
  state.missions = saved.missions || [
    { title: "Start local Python core", status: "pending", priority: "high", source: "browser" },
  ];
  state.status = {
    version: "0.1.0",
    uptime_seconds: 0,
    python: "offline",
    host: "browser",
    port: "local",
    workspace: "not connected",
    tool_policy: "safe-actions-only",
  };
}

function saveFallbackState() {
  localStorage.setItem(
    "kaine-fallback",
    JSON.stringify({
      transcript: state.transcript.slice(-40),
      memories: state.memories.slice(0, 20),
      missions: state.missions.slice(0, 20),
    }),
  );
}

async function sendMessage(message) {
  const text = message.trim();
  if (!text) return;
  els.promptInput.value = "";
  setSpeaking(false);

  if (!state.online) {
    state.transcript.push({ role: "user", content: text });
    const reply = fallbackReply(text);
    state.transcript.push({ role: "assistant", content: reply });
    if (text.toLowerCase().includes("remember")) {
      state.memories.unshift({ title: text.slice(0, 80), body: text, kind: "browser" });
    }
    saveFallbackState();
    renderAll();
    speak(reply);
    return;
  }

  state.transcript.push({ role: "user", content: text });
  state.transcript.push({ role: "assistant", content: "Processing directive..." });
  renderTranscript();

  try {
    const payload = await api.post("/api/chat", { message: text, mode: state.mode });
    applyServerState(payload.state);
    speak(payload.result.reply);
  } catch (error) {
    state.transcript.pop();
    const reply = `Local transport fault: ${error.message}. Browser fallback remains active.`;
    state.transcript.push({ role: "assistant", content: reply });
    setOnline(false);
    saveFallbackState();
    renderAll();
    speak(reply);
  }
}

function fallbackReply(text) {
  const lower = text.toLowerCase();
  if (lower.includes("status")) {
    return "Fallback status: UI active, server disconnected, no external tools available.";
  }
  if (lower.includes("security")) {
    return "Fallback security posture: keep actions local, require explicit approvals, and do not expose shell tools directly from chat.";
  }
  if (lower.includes("remember")) {
    return "Fallback memory captured in browser storage. Start the server for durable project memory.";
  }
  return "Fallback response: directive received. Start the local Python core for persistent memory, missions, and diagnostics.";
}

function setSpeaking(active) {
  state.speaking = active;
  els.core.classList.toggle("speaking", active);
}

function speak(text) {
  if (!els.speakToggle.checked || !("speechSynthesis" in window)) return;
  window.speechSynthesis.cancel();
  const utterance = new SpeechSynthesisUtterance(text.replace(/\n+/g, ". "));
  utterance.rate = 0.96;
  utterance.pitch = 0.82;
  utterance.volume = 0.88;
  utterance.onstart = () => setSpeaking(true);
  utterance.onend = () => setSpeaking(false);
  utterance.onerror = () => setSpeaking(false);
  window.speechSynthesis.speak(utterance);
}

function setupRecognition() {
  const Recognition = window.SpeechRecognition || window.webkitSpeechRecognition;
  if (!Recognition) {
    els.listenButton.disabled = true;
    els.listenButton.title = "Voice input unavailable in this browser";
    return;
  }
  state.recognition = new Recognition();
  state.recognition.continuous = false;
  state.recognition.interimResults = false;
  state.recognition.lang = "en-US";
  state.recognition.onstart = () => setSpeaking(true);
  state.recognition.onend = () => setSpeaking(false);
  state.recognition.onerror = () => setSpeaking(false);
  state.recognition.onresult = (event) => {
    const transcript = Array.from(event.results)
      .map((result) => result[0]?.transcript || "")
      .join(" ")
      .trim();
    if (transcript) {
      els.promptInput.value = transcript;
      sendMessage(transcript);
    }
  };
}

async function createManualMission() {
  const title = els.promptInput.value.trim() || "Expand Kaine capability layer";
  if (!state.online) {
    state.missions.unshift({ title, status: "active", priority: "normal", source: "browser" });
    saveFallbackState();
    renderAll();
    return;
  }
  const payload = await api.post("/api/mission", { title, priority: "normal", source: "manual" });
  applyServerState(payload.state);
}

async function saveManualMemory() {
  const body = els.promptInput.value.trim();
  if (!body) return;
  if (!state.online) {
    state.memories.unshift({ title: body.slice(0, 80), body, kind: "browser" });
    saveFallbackState();
    renderAll();
    return;
  }
  const payload = await api.post("/api/memory", { title: body.slice(0, 90), body });
  applyServerState(payload.state);
}

function setupEvents() {
  document.querySelectorAll(".mode").forEach((button) => {
    button.addEventListener("click", () => {
      state.mode = button.dataset.mode;
      document.querySelectorAll(".mode").forEach((node) => node.classList.toggle("active", node === button));
      renderAll();
    });
  });

  els.composer.addEventListener("submit", (event) => {
    event.preventDefault();
    sendMessage(els.promptInput.value);
  });

  els.promptInput.addEventListener("keydown", (event) => {
    if (event.key === "Enter" && (event.ctrlKey || event.metaKey)) {
      event.preventDefault();
      sendMessage(els.promptInput.value);
    }
  });

  document.querySelectorAll(".quickbar button").forEach((button) => {
    button.addEventListener("click", () => sendMessage(button.dataset.prompt || ""));
  });

  els.listenButton.addEventListener("click", () => {
    if (state.recognition) state.recognition.start();
  });

  els.speakToggle.addEventListener("change", () => {
    if (!els.speakToggle.checked && "speechSynthesis" in window) {
      window.speechSynthesis.cancel();
      setSpeaking(false);
    }
  });

  els.missionButton.addEventListener("click", () => createManualMission());
  els.memoryButton.addEventListener("click", () => saveManualMemory());
}

function startClock() {
  const update = () => {
    els.clockLabel.textContent = new Intl.DateTimeFormat([], {
      hour: "numeric",
      minute: "2-digit",
    }).format(new Date());
  };
  update();
  setInterval(update, 15_000);
}

function startField() {
  const canvas = els.field;
  const ctx = canvas.getContext("2d");
  const points = [];

  function resize() {
    canvas.width = window.innerWidth * window.devicePixelRatio;
    canvas.height = window.innerHeight * window.devicePixelRatio;
    canvas.style.width = `${window.innerWidth}px`;
    canvas.style.height = `${window.innerHeight}px`;
    ctx.setTransform(window.devicePixelRatio, 0, 0, window.devicePixelRatio, 0, 0);
    points.length = 0;
    const count = Math.min(84, Math.floor((window.innerWidth * window.innerHeight) / 16000));
    for (let i = 0; i < count; i += 1) {
      points.push({
        x: Math.random() * window.innerWidth,
        y: Math.random() * window.innerHeight,
        vx: (Math.random() - 0.5) * 0.18,
        vy: (Math.random() - 0.5) * 0.18,
      });
    }
  }

  function draw() {
    ctx.clearRect(0, 0, window.innerWidth, window.innerHeight);
    ctx.lineWidth = 1;
    points.forEach((point, index) => {
      point.x += point.vx;
      point.y += point.vy;
      if (point.x < 0 || point.x > window.innerWidth) point.vx *= -1;
      if (point.y < 0 || point.y > window.innerHeight) point.vy *= -1;

      for (let j = index + 1; j < points.length; j += 1) {
        const other = points[j];
        const dx = point.x - other.x;
        const dy = point.y - other.y;
        const distance = Math.hypot(dx, dy);
        if (distance < 150) {
          const alpha = (1 - distance / 150) * 0.16;
          ctx.strokeStyle = `rgba(88, 224, 212, ${alpha})`;
          ctx.beginPath();
          ctx.moveTo(point.x, point.y);
          ctx.lineTo(other.x, other.y);
          ctx.stroke();
        }
      }
    });
    requestAnimationFrame(draw);
  }

  window.addEventListener("resize", resize);
  resize();
  draw();
}

setupEvents();
setupRecognition();
startClock();
startField();
loadState();
