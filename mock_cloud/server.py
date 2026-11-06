"""Mock cloud for the Solar Health Monitor.

Mimics ThingSpeak's /update endpoint so the firmware can talk to it with no
internet account. To switch to real ThingSpeak later, only CLOUD_URL in
firmware/solar_health_monitor/config.h changes (plus &api_key=...).

Run:      python mock_cloud/server.py          -> http://localhost:5000
Dashboard: http://localhost:5000/              (auto-refreshes every 5 s)
Raw data:  http://localhost:5000/data          (JSON)
"""
import threading
from collections import deque

from flask import Flask, Response, jsonify, request

app = Flask(__name__)

MAX_POINTS = 1000
history = deque(maxlen=MAX_POINTS)

# ThingSpeak-compatible field mapping
FIELDS = ("field1", "field2", "field3", "field4", "field5", "field6", "field7")
KEYS = ("v", "i", "p", "lux", "temp", "soil", "state")


@app.route("/update", methods=["GET", "POST"])
def update():
    """Accept 7 fields as query params (GET, what the firmware uses) or form."""
    src = request.values
    rec = {}
    for field, key in zip(FIELDS, KEYS):
        raw = src.get(field)
        if raw is None:
            rec[key] = None
            continue
        try:
            rec[key] = float(raw)
        except ValueError:
            return Response("bad value for " + field, status=400)
    if rec["state"] is not None:
        rec["state"] = int(rec["state"])
    rec["ts"] = __import__("datetime").datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    history.append(rec)
    return Response("ok", status=200)


@app.route("/data")
def data():
    return jsonify(list(history))


@app.route("/")
def index():
    return Response(INDEX_HTML, mimetype="text/html")


INDEX_HTML = """<!doctype html>
<title>Solar Health Monitor - mock cloud</title>
<meta charset="utf-8">
<style>
 body{font-family:Segoe UI,Arial,sans-serif;margin:20px;background:#f4f7fb;color:#123}
 h1{font-size:20px;color:#1F4E79}
 table{border-collapse:collapse;font-size:13px;background:#fff}
 th,td{border:1px solid #c9d6e5;padding:4px 8px;text-align:right}
 th{background:#1F4E79;color:#fff}
 td.alert{background:#ffd6d6;font-weight:bold}
 #state{font-size:18px;font-weight:bold}
</style>
<h1>Solar Health Monitor &mdash; mock cloud</h1>
<p>Status: <span id="state">-</span> &nbsp; Points: <span id="n">0</span></p>
<table id="t"><thead><tr>
 <th>time</th><th>V</th><th>mA</th><th>W</th><th>lux</th><th>&deg;C</th>
 <th>soiling %</th><th>state</th>
</tr></thead><tbody></tbody></table>
<script>
const NAMES=['-', 'CLEANING REQUIRED', 'FAULT', 'OVERHEAT'];
async function tick(){
  try{
    const r = await fetch('/data'); const d = await r.json();
    document.getElementById('n').textContent = d.length;
    const last = d[d.length-1];
    const s = document.getElementById('state');
    if(last){ s.textContent = NAMES[last.state] || last.state;
              s.className = last.state ? 'alert' : 'ok'; }
    const rows = d.slice(-25).reverse().map(x =>
      '<tr class="'+(x.state?'alert':'')+'"><td>'+x.ts+'</td><td>'+x.v+
      '</td><td>'+x.i+'</td><td>'+x.p+'</td><td>'+x.lux+'</td><td>'+x.temp+
      '</td><td>'+x.soil+'</td><td>'+(NAMES[x.state]||x.state)+'</td></tr>');
    document.querySelector('#t tbody').innerHTML = rows.join('');
  }catch(e){ document.getElementById('state').textContent='server unreachable'; }
}
tick(); setInterval(tick, 5000);
</script>
"""

if __name__ == "__main__":
    threading.Event()  # keep import of threading meaningful for future use
    print("mock cloud: http://localhost:5000  (dashboard)  /update  /data")
    app.run(host="0.0.0.0", port=5000)
