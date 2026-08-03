from flask import Flask, render_template

app = Flask(__name__)


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


@app.errorhandler(404)
def not_found(_error):
    return render_template("404.html"), 404


if __name__ == "__main__":
    app.run(host="127.0.0.1", port=5000, debug=True)
