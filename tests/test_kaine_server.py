import json
import tempfile
import threading
import time
import unittest
from pathlib import Path
from urllib.request import Request, urlopen

from kaine_server import create_server


class KaineServerTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.web = self.root / "web"
        self.web.mkdir()
        (self.web / "index.html").write_text("<html>Kaine</html>", encoding="utf-8")
        self.server = create_server(
            root=self.root,
            web_root=self.web,
            data_dir=self.root / "data",
            host="127.0.0.1",
            port=0,
        )
        self.thread = threading.Thread(target=self.server.serve_forever, daemon=True)
        self.thread.start()
        self.base = f"http://127.0.0.1:{self.server.server_port}"

    def tearDown(self):
        self.server.shutdown()
        self.server.server_close()
        self.thread.join(timeout=2)
        self.tmp.cleanup()

    def get_json(self, path):
        with urlopen(self.base + path, timeout=5) as response:
            return json.loads(response.read().decode("utf-8"))

    def post_json(self, path, payload):
        request = Request(
            self.base + path,
            data=json.dumps(payload).encode("utf-8"),
            headers={"Content-Type": "application/json"},
            method="POST",
        )
        with urlopen(request, timeout=5) as response:
            return json.loads(response.read().decode("utf-8"))

    def test_status_endpoint_reports_online_core(self):
        payload = self.get_json("/api/status")
        self.assertTrue(payload["online"])
        self.assertEqual(payload["name"], "Kaine")
        self.assertEqual(payload["tool_policy"], "safe-actions-only")

    def test_chat_persists_memory_when_user_requests_it(self):
        payload = self.post_json(
            "/api/chat",
            {"mode": "Operator", "message": "remember that Kaine should never require cloud keys to launch"},
        )
        self.assertIn("assistant_message", payload)
        self.assertGreaterEqual(len(payload["state"]["memories"]), 1)
        self.assertIn("cloud keys", payload["state"]["memories"][0]["body"])

    def test_manual_mission_is_created(self):
        payload = self.post_json(
            "/api/mission",
            {"title": "Add permissioned project tools", "priority": "high", "source": "test"},
        )
        self.assertEqual(payload["mission"]["title"], "Add permissioned project tools")
        self.assertEqual(payload["mission"]["priority"], "high")


if __name__ == "__main__":
    unittest.main()
