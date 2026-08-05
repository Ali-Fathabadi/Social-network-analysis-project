from __future__ import annotations

import json
import os
import subprocess
from pathlib import Path
from typing import Any

from flask import Flask, jsonify, render_template, request

app = Flask(__name__)

UI_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = UI_DIR.parent
DATABASE_PATH = Path(
    os.environ.get("SOCIAL_NETWORK_DB", PROJECT_ROOT / "network.json")
).expanduser().resolve()


def locate_cli() -> Path:
    configured = os.environ.get("SOCIAL_NETWORK_CLI")
    candidates: list[Path] = []
    if configured:
        candidates.append(Path(configured).expanduser())
    candidates.extend(
        [
            PROJECT_ROOT / "main.exe",
            PROJECT_ROOT / "main",
            PROJECT_ROOT / "x64" / "Release" / "main.exe",
            PROJECT_ROOT / "Release" / "main.exe",
            PROJECT_ROOT / "build" / "Release" / "main.exe",
            PROJECT_ROOT / "build" / "main.exe",
            PROJECT_ROOT / "build" / "main",
        ]
    )
    for candidate in candidates:
        path = candidate if candidate.is_absolute() else PROJECT_ROOT / candidate
        if path.is_file():
            return path.resolve()
    raise FileNotFoundError(
        "CLI executable was not found. Compile the C++ files and place main.exe "
        "(Windows) or main (Linux) in the project root, or set SOCIAL_NETWORK_CLI."
    )


def run_cli(*arguments: object, timeout: int = 60) -> tuple[dict[str, Any], int]:
    try:
        executable = locate_cli()
    except FileNotFoundError as exc:
        return {"status": "error", "message": str(exc)}, 503

    environment = os.environ.copy()
    environment["SOCIAL_NETWORK_DB"] = str(DATABASE_PATH)
    command = [str(executable), *(str(argument) for argument in arguments)]

    try:
        completed = subprocess.run(
            command,
            cwd=PROJECT_ROOT,
            env=environment,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=timeout,
            check=False,
        )
    except subprocess.TimeoutExpired:
        return {"status": "error", "message": "The operation exceeded its time limit."}, 504
    except OSError as exc:
        return {"status": "error", "message": f"Could not start CLI: {exc}"}, 503

    output = completed.stdout.strip()
    try:
        payload = json.loads(output) if output else {}
    except json.JSONDecodeError:
        message = completed.stderr.strip() or output or "CLI returned an invalid response."
        return {"status": "error", "message": message}, 502

    if not isinstance(payload, dict):
        return {"status": "error", "message": "CLI response must be a JSON object."}, 502
    if completed.returncode != 0 or payload.get("status") == "error":
        return payload, 500 if completed.returncode == 2 else 400
    return payload, 200


def cli_response(*arguments: object, timeout: int = 60):
    payload, status = run_cli(*arguments, timeout=timeout)
    return jsonify(payload), status


def required_json_fields(*names: str) -> tuple[list[str] | None, tuple[Any, int] | None]:
    body = request.get_json(silent=True)
    if not isinstance(body, dict):
        return None, (jsonify(status="error", message="A JSON object is required."), 400)
    values: list[str] = []
    for name in names:
        value = body.get(name)
        if value is None or not str(value).strip():
            return None, (jsonify(status="error", message=f"Field '{name}' is required."), 400)
        values.append(str(value).strip())
    return values, None


# ---------------- Page routes ----------------
@app.get("/")
def dashboard():
    return render_template("dashboard.html")


@app.get("/users")
def users():
    return render_template("users.html")


@app.get("/paths")
def paths():
    return render_template("paths.html")


@app.get("/distances")
def distances():
    return render_template("distances.html")


@app.get("/management")
def management():
    return render_template("management.html")


@app.get("/groups")
def groups():
    return render_template("groups.html")


@app.get("/graph")
def graph():
    return render_template("graph.html")


@app.get("/insights")
def insights():
    return render_template("insights.html")


# ---------------- JSON API ----------------
@app.get("/api/statistics")
def api_statistics():
    return cli_response("networkStatistics")


@app.get("/api/users")
def api_list_users():
    return cli_response("listUsers")


@app.post("/api/users")
def api_add_user():
    values, error = required_json_fields("id", "name")
    return error if error else cli_response("addUser", *values)


@app.get("/api/users/<path:user_id>")
def api_get_user(user_id: str):
    return cli_response("getUser", user_id)


@app.put("/api/users/<path:user_id>")
def api_edit_user(user_id: str):
    values, error = required_json_fields("name")
    return error if error else cli_response("editUser", user_id, values[0])


@app.delete("/api/users/<path:user_id>")
def api_remove_user(user_id: str):
    return cli_response("removeUser", user_id)


@app.post("/api/friendships")
def api_add_friendship():
    values, error = required_json_fields("id1", "id2")
    return error if error else cli_response("addFriendship", *values)


@app.delete("/api/friendships")
def api_remove_friendship():
    values, error = required_json_fields("id1", "id2")
    return error if error else cli_response("removeFriendship", *values)


@app.get("/api/friends/<path:user_id>")
def api_friends(user_id: str):
    return cli_response("getFriends", user_id)


@app.get("/api/recommendations/<path:user_id>")
def api_recommendations(user_id: str):
    return cli_response("recommendFriends", user_id)


@app.get("/api/mutual")
def api_mutual():
    first = request.args.get("id1", "").strip()
    second = request.args.get("id2", "").strip()
    if not first or not second:
        return jsonify(status="error", message="id1 and id2 are required."), 400
    return cli_response("mutualFriends", first, second)


@app.get("/api/path")
def api_path():
    source = request.args.get("source", "").strip()
    target = request.args.get("target", "").strip()
    if not source or not target:
        return jsonify(status="error", message="source and target are required."), 400

    direct, status = run_cli("areFriends", source, target)
    if status != 200:
        return jsonify(direct), status
    connected, status = run_cli("isConnected", source, target)
    if status != 200:
        return jsonify(connected), status
    shortest, status = run_cli("shortestPath", source, target)
    if status != 200:
        return jsonify(shortest), status

    return jsonify(
        status="success",
        are_friends=direct["are_friends"],
        connected=connected["connected"],
        path=shortest["path"],
        distance=shortest["distance"],
    )


@app.get("/api/distances/<path:user_id>")
def api_distances(user_id: str):
    return cli_response("distanceFromUser", user_id)


@app.get("/api/components")
def api_components():
    return cli_response("findConnectedComponents")


@app.get("/api/ranking")
def api_ranking():
    return cli_response("degreeRanking")


@app.get("/api/key-users")
def api_key_users():
    return cli_response("keyUserRanking", timeout=120)


@app.get("/api/communities")
def api_communities():
    return cli_response("communityDetection", timeout=120)


@app.get("/api/news-spread")
def api_news_spread():
    k = request.args.get("k", "").strip()
    if not k:
        return jsonify(status="error", message="k is required."), 400
    return cli_response("optimizeNewsSpread", k, timeout=120)


@app.get("/api/graph")
def api_graph():
    return cli_response("graphData")


@app.get("/api/health")
def api_health():
    payload, status = run_cli("networkStatistics")
    return jsonify(status="success" if status == 200 else "error", cli=payload), status


@app.errorhandler(404)
def not_found(_error):
    if request.path.startswith("/api/"):
        return jsonify(status="error", message="API endpoint not found."), 404
    return render_template("404.html"), 404


if __name__ == "__main__":
    DATABASE_PATH.parent.mkdir(parents=True, exist_ok=True)
    app.run(host="127.0.0.1", port=5000, debug=False, threaded=True)
