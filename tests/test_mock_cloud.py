"""Unit tests for the mock cloud (write BEFORE implementing server.py).

Run (from repo root):
    python -m unittest discover -s tests -p "test_*.py"

Uses Flask's in-memory test client, so no port/socket is needed.
"""
import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "mock_cloud"))

from server import app, history  # noqa: E402  (fails here until server.py exists)


class MockCloudTest(unittest.TestCase):
    def setUp(self):
        history.clear()
        self.client = app.test_client()

    def _payload(self, **overrides):
        data = {
            "field1": "6.01",    # V test
            "field2": "486",     # mA test
            "field3": "2.92",    # W test
            "field4": "42150",   # lux
            "field5": "41.6",    # degC
            "field6": "1.6",     # soiling %
            "field7": "0",       # state
        }
        data.update(overrides)
        return data

    def test_post_form_fields_are_recorded(self):
        r = self.client.post("/update", data=self._payload())
        self.assertEqual(r.status_code, 200)
        data = self.client.get("/data").get_json()
        self.assertEqual(len(data), 1)
        self.assertAlmostEqual(data[0]["v"], 6.01)
        self.assertAlmostEqual(data[0]["i"], 486.0)
        self.assertAlmostEqual(data[0]["soil"], 1.6)
        self.assertEqual(data[0]["state"], 0)

    def test_get_query_style_upload_works(self):
        # firmware uses GET with query params (ThingSpeak-compatible style)
        r = self.client.get("/update?field1=5.8&field2=400&field3=2.32"
                            "&field4=40000&field5=41&field6=20&field7=1")
        self.assertEqual(r.status_code, 200)
        data = self.client.get("/data").get_json()
        self.assertEqual(len(data), 1)
        self.assertAlmostEqual(data[0]["soil"], 20.0)
        self.assertEqual(data[0]["state"], 1)

    def test_history_appends_in_order(self):
        self.client.post("/update", data=self._payload(field6="0"))
        self.client.post("/update", data=self._payload(field6="20"))
        data = self.client.get("/data").get_json()
        self.assertEqual([d["soil"] for d in data], [0.0, 20.0])

    def test_timestamp_present(self):
        self.client.post("/update", data=self._payload())
        data = self.client.get("/data").get_json()
        self.assertIn("ts", data[0])

    def test_non_numeric_field_rejected(self):
        r = self.client.post("/update", data=self._payload(field1="garbage"))
        self.assertEqual(r.status_code, 400)
        self.assertEqual(len(self.client.get("/data").get_json()), 0)

    def test_index_page_serves_dashboard(self):
        r = self.client.get("/")
        self.assertEqual(r.status_code, 200)
        self.assertIn(b"Solar", r.data)


if __name__ == "__main__":
    unittest.main()
