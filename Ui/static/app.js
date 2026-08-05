"use strict";

function showToast(message, kind = "info") {
    const toast = document.getElementById("toast");
    if (!toast) return;
    toast.textContent = message;
    toast.dataset.kind = kind;
    toast.classList.add("visible");
    window.clearTimeout(showToast.timer);
    showToast.timer = window.setTimeout(() => toast.classList.remove("visible"), 3000);
}

async function api(url, options = {}) {
    const response = await fetch(url, {
        headers: {"Content-Type": "application/json", ...(options.headers || {})},
        ...options,
    });
    let payload;
    try {
        payload = await response.json();
    } catch (_error) {
        throw new Error(`Server returned a non-JSON response (${response.status}).`);
    }
    if (!response.ok || payload.status === "error") {
        throw new Error(payload.message || `Request failed (${response.status}).`);
    }
    return payload;
}

function setResult(id, message, kind = "success") {
    const element = document.getElementById(id);
    if (!element) return;
    element.textContent = message;
    element.dataset.kind = kind;
    element.classList.add("visible");
}

function clearElement(element) {
    while (element?.firstChild) element.removeChild(element.firstChild);
}

function makeTag(text) {
    const span = document.createElement("span");
    span.className = "tag";
    span.textContent = text;
    return span;
}

function makeEmpty(text) {
    const div = document.createElement("div");
    div.className = "empty-state";
    div.textContent = text;
    return div;
}

function yesNo(value) {
    return value ? "Yes" : "No";
}

function formatNumber(value, digits = 2) {
    const number = Number(value);
    return Number.isFinite(number) ? number.toFixed(digits).replace(/\.00$/, "") : "—";
}

function setupMobileMenu() {
    const button = document.getElementById("mobile-menu");
    const sidebar = document.getElementById("sidebar");
    if (!button || !sidebar) return;
    button.addEventListener("click", () => {
        const open = sidebar.classList.toggle("open");
        button.setAttribute("aria-expanded", String(open));
    });
}

async function loadDashboard() {
    if (!document.getElementById("stats-total-users")) return;
    const status = document.getElementById("dashboard-status");
    if (status) status.textContent = "Loading live data from the C++ core...";
    try {
        const data = await api("/api/statistics");
        document.getElementById("stats-total-users").textContent = data.total_users;
        document.getElementById("stats-total-edges").textContent = data.total_edges;
        document.getElementById("stats-average-friends").textContent = formatNumber(data.avg_friends);
        document.getElementById("stats-largest-group").textContent = data.largest_comp_size;
        if (status) {
            status.textContent = data.total_users
                ? `Most connected: ${data.most_connected_id} (${data.most_connected_count} friends)`
                : "The network is empty. Add users from the Management page.";
        }
    } catch (error) {
        if (status) status.textContent = error.message;
        showToast(error.message, "error");
    }
}

function renderUserDetails(user) {
    const container = document.getElementById("user-details");
    if (!container) return;
    clearElement(container);
    container.className = "node-details";

    const title = document.createElement("strong");
    title.textContent = `${user.name} (${user.id})`;
    container.appendChild(title);

    const count = document.createElement("span");
    count.className = "detail-line";
    count.textContent = `${user.friends.length} friend(s)`;
    container.appendChild(count);

    const tags = document.createElement("div");
    tags.className = "placeholder-list";
    if (user.friends.length) user.friends.forEach((id) => tags.appendChild(makeTag(id)));
    else tags.appendChild(makeEmpty("This user has no friends."));
    container.appendChild(tags);
}

function renderSuggestions(suggestions) {
    const container = document.getElementById("user-suggestions");
    if (!container) return;
    clearElement(container);
    container.className = "node-details";
    if (!suggestions.length) {
        container.appendChild(makeEmpty("No friend suggestion is currently available."));
        return;
    }
    suggestions.forEach((suggestion) => {
        const row = document.createElement("div");
        row.className = "detail-line";
        row.textContent = `${suggestion.id} - ${suggestion.mutual_count} mutual friend(s)`;
        container.appendChild(row);
    });
}

function setupUsersPage() {
    const searchForm = document.getElementById("user-search-form");
    searchForm?.addEventListener("submit", async (event) => {
        event.preventDefault();
        const id = document.getElementById("user-id").value.trim();
        if (!id) return showToast("Enter a user ID.", "error");
        setResult("user-search-result", "Loading user and recommendations...", "info");
        try {
            const [user, recommendations] = await Promise.all([
                api(`/api/users/${encodeURIComponent(id)}`),
                api(`/api/recommendations/${encodeURIComponent(id)}`),
            ]);
            renderUserDetails(user);
            renderSuggestions(recommendations.suggestions);
            setResult("user-search-result", `User ${id} loaded successfully.`);
        } catch (error) {
            setResult("user-search-result", error.message, "error");
            showToast(error.message, "error");
        }
    });

    const mutualForm = document.getElementById("mutual-form");
    mutualForm?.addEventListener("submit", async (event) => {
        event.preventDefault();
        const first = document.getElementById("mutual-user-one").value.trim();
        const second = document.getElementById("mutual-user-two").value.trim();
        if (!first || !second) return showToast("Enter both user IDs.", "error");
        try {
            const data = await api(`/api/mutual?id1=${encodeURIComponent(first)}&id2=${encodeURIComponent(second)}`);
            const text = data.mutual_friends.length
                ? `Mutual friends: ${data.mutual_friends.join(", ")}`
                : "These users have no mutual friends.";
            setResult("mutual-result", text);
        } catch (error) {
            setResult("mutual-result", error.message, "error");
        }
    });
}

function setupPathsPage() {
    const form = document.getElementById("path-form");
    if (!form) return;
    form.addEventListener("submit", async (event) => {
        event.preventDefault();
        const source = document.getElementById("source-id").value.trim();
        const target = document.getElementById("target-id").value.trim();
        if (!source || !target) return showToast("Enter source and target IDs.", "error");
        try {
            const data = await api(`/api/path?source=${encodeURIComponent(source)}&target=${encodeURIComponent(target)}`);
            document.getElementById("path-direct").textContent = yesNo(data.are_friends);
            document.getElementById("path-connected").textContent = yesNo(data.connected);
            document.getElementById("path-distance").textContent = data.distance ?? "∞";

            const preview = document.getElementById("path-preview");
            clearElement(preview);
            if (!data.path.length) {
                preview.appendChild(makeEmpty("No path exists between these users."));
            } else {
                data.path.forEach((id, index) => {
                    const node = document.createElement("span");
                    node.className = "path-node";
                    node.textContent = id;
                    preview.appendChild(node);
                    if (index + 1 < data.path.length) {
                        const arrow = document.createElement("span");
                        arrow.textContent = "→";
                        preview.appendChild(arrow);
                    }
                });
            }
            setResult("path-result", data.connected ? "Shortest path calculated." : "Users are disconnected.");
        } catch (error) {
            setResult("path-result", error.message, "error");
        }
    });
}

function renderDistances(distances) {
    const body = document.getElementById("distance-table-body");
    if (!body) return;
    clearElement(body);
    if (!distances.length) {
        const row = body.insertRow();
        const cell = row.insertCell();
        cell.colSpan = 3;
        cell.className = "empty-state";
        cell.textContent = "There are no other users in the network.";
        return;
    }
    distances.forEach((entry) => {
        const row = body.insertRow();
        row.insertCell().textContent = entry.id;
        row.insertCell().textContent = entry.distance === null ? "∞" : entry.distance;
        row.insertCell().textContent = entry.distance === null ? "Unreachable" : "Reachable";
    });
}

function setupDistancesPage() {
    const form = document.getElementById("distance-form");
    form?.addEventListener("submit", async (event) => {
        event.preventDefault();
        const id = document.getElementById("distance-user-id").value.trim();
        if (!id) return showToast("Enter a user ID.", "error");
        try {
            const data = await api(`/api/distances/${encodeURIComponent(id)}`);
            renderDistances(data.distances);
            setResult("distance-result", `Distances from ${id} were calculated.`);
        } catch (error) {
            setResult("distance-result", error.message, "error");
        }
    });
}

async function loadUserTable() {
    const body = document.getElementById("user-table-body");
    if (!body) return;
    clearElement(body);
    try {
        const data = await api("/api/users");
        if (!data.users.length) {
            const row = body.insertRow();
            const cell = row.insertCell();
            cell.colSpan = 4;
            cell.className = "empty-state";
            cell.textContent = "No users have been added.";
            return;
        }
        data.users.forEach((user) => {
            const row = body.insertRow();
            row.insertCell().textContent = user.id;
            row.insertCell().textContent = user.name;
            row.insertCell().textContent = user.friend_count;
            const action = row.insertCell();
            const button = document.createElement("button");
            button.className = "button ghost compact-button";
            button.type = "button";
            button.textContent = "Delete";
            button.addEventListener("click", async () => {
                if (!window.confirm(`Remove user ${user.id}?`)) return;
                try {
                    await api(`/api/users/${encodeURIComponent(user.id)}`, {method: "DELETE"});
                    showToast(`User ${user.id} removed.`, "success");
                    await loadUserTable();
                } catch (error) {
                    showToast(error.message, "error");
                }
            });
            action.appendChild(button);
        });
    } catch (error) {
        const row = body.insertRow();
        const cell = row.insertCell();
        cell.colSpan = 4;
        cell.className = "empty-state";
        cell.textContent = error.message;
    }
}

function bindJsonForm(formId, resultId, requestFactory, successMessage) {
    const form = document.getElementById(formId);
    form?.addEventListener("submit", async (event) => {
        event.preventDefault();
        try {
            const {url, options} = requestFactory();
            const data = await api(url, options);
            setResult(resultId, data.message || successMessage);
            form.reset();
            await loadUserTable();
            showToast(data.message || successMessage, "success");
        } catch (error) {
            setResult(resultId, error.message, "error");
            showToast(error.message, "error");
        }
    });
}

function setupManagementPage() {
    if (!document.getElementById("user-table-body")) return;

    bindJsonForm("add-user-form", "add-user-result", () => ({
        url: "/api/users",
        options: {
            method: "POST",
            body: JSON.stringify({
                id: document.getElementById("add-user-id").value.trim(),
                name: document.getElementById("add-user-name").value.trim(),
            }),
        },
    }), "User added.");

    bindJsonForm("edit-user-form", "edit-user-result", () => ({
        url: `/api/users/${encodeURIComponent(document.getElementById("edit-user-id").value.trim())}`,
        options: {
            method: "PUT",
            body: JSON.stringify({name: document.getElementById("edit-user-name").value.trim()}),
        },
    }), "User updated.");

    bindJsonForm("remove-user-form", "remove-user-result", () => ({
        url: `/api/users/${encodeURIComponent(document.getElementById("remove-user-id").value.trim())}`,
        options: {method: "DELETE"},
    }), "User removed.");

    bindJsonForm("add-friendship-form", "add-friendship-result", () => ({
        url: "/api/friendships",
        options: {
            method: "POST",
            body: JSON.stringify({
                id1: document.getElementById("add-friend-one").value.trim(),
                id2: document.getElementById("add-friend-two").value.trim(),
            }),
        },
    }), "Friendship added.");

    bindJsonForm("remove-friendship-form", "remove-friendship-result", () => ({
        url: "/api/friendships",
        options: {
            method: "DELETE",
            body: JSON.stringify({
                id1: document.getElementById("remove-friend-one").value.trim(),
                id2: document.getElementById("remove-friend-two").value.trim(),
            }),
        },
    }), "Friendship removed.");

    document.getElementById("refresh-user-table")?.addEventListener("click", loadUserTable);
    loadUserTable();
}

function renderGroups(components) {
    const container = document.getElementById("groups-result");
    if (!container) return;
    clearElement(container);
    if (!components.length) {
        container.appendChild(makeEmpty("The network is empty."));
        return;
    }
    components.forEach((members, index) => {
        const article = document.createElement("article");
        article.className = "state-card";
        const title = document.createElement("strong");
        title.textContent = `Group ${index + 1} (${members.length})`;
        article.appendChild(title);
        const tags = document.createElement("div");
        tags.className = "placeholder-list";
        members.forEach((id) => tags.appendChild(makeTag(id)));
        article.appendChild(tags);
        container.appendChild(article);
    });
}

function renderRanking(users) {
    const body = document.getElementById("ranking-table-body");
    if (!body) return;
    clearElement(body);
    if (!users.length) {
        const row = body.insertRow();
        const cell = row.insertCell();
        cell.colSpan = 4;
        cell.className = "empty-state";
        cell.textContent = "The network is empty.";
        return;
    }
    users.forEach((user, index) => {
        const row = body.insertRow();
        row.insertCell().textContent = index + 1;
        row.insertCell().textContent = user.id;
        row.insertCell().textContent = user.name;
        row.insertCell().textContent = user.friend_count;
    });
}

async function loadGroupsPage() {
    if (!document.getElementById("groups-result")) return;
    try {
        const [components, ranking, stats] = await Promise.all([
            api("/api/components"),
            api("/api/ranking"),
            api("/api/statistics"),
        ]);
        renderGroups(components.components);
        renderRanking(ranking.users);
        document.getElementById("component-count").textContent = components.components.length;
        document.getElementById("largest-component-size").textContent = stats.largest_comp_size;
        document.getElementById("most-connected-user").textContent = stats.most_connected_id
            ? `${stats.most_connected_id} (${stats.most_connected_count})`
            : "—";
    } catch (error) {
        const container = document.getElementById("groups-result");
        clearElement(container);
        container.appendChild(makeEmpty(error.message));
        showToast(error.message, "error");
    }
}

function setupGroupsPage() {
    if (!document.getElementById("groups-result")) return;
    document.getElementById("load-groups")?.addEventListener("click", loadGroupsPage);
    loadGroupsPage();
}

function renderKeyUsers(data) {
    const container = document.getElementById("key-users-result");
    if (!container) return;
    clearElement(container);
    container.classList.add("visible");
    if (!data.key_users.length) {
        container.appendChild(makeEmpty("No bridge-like key user exists in the current graph."));
        return;
    }
    const keySet = new Set(data.key_users);
    data.ranking.filter((entry) => keySet.has(entry.id)).forEach((entry) => {
        const line = document.createElement("div");
        line.className = "detail-line";
        line.textContent = `${entry.id} (${entry.name}) - score ${formatNumber(entry.score, 4)}`;
        container.appendChild(line);
    });
}

function renderCommunities(communities) {
    const container = document.getElementById("community-result");
    if (!container) return;
    clearElement(container);
    container.classList.add("visible");
    if (!communities.length) {
        container.appendChild(makeEmpty("The network is empty."));
        return;
    }
    communities.forEach((community) => {
        const line = document.createElement("div");
        line.className = "detail-line";
        line.textContent = `Community ${community.id}: ${community.members.join(", ")}`;
        container.appendChild(line);
    });
}

function setupInsightsPage() {
    const keyButton = document.getElementById("key-users-button");
    keyButton?.addEventListener("click", async () => {
        keyButton.disabled = true;
        setResult("key-users-result", "Calculating betweenness centrality...", "info");
        try {
            renderKeyUsers(await api("/api/key-users"));
        } catch (error) {
            setResult("key-users-result", error.message, "error");
        } finally {
            keyButton.disabled = false;
        }
    });

    const communityButton = document.getElementById("community-button");
    communityButton?.addEventListener("click", async () => {
        communityButton.disabled = true;
        setResult("community-result", "Detecting communities...", "info");
        try {
            renderCommunities((await api("/api/communities")).communities);
        } catch (error) {
            setResult("community-result", error.message, "error");
        } finally {
            communityButton.disabled = false;
        }
    });

    const spreadForm = document.getElementById("spread-form");
    spreadForm?.addEventListener("submit", async (event) => {
        event.preventDefault();
        const k = document.getElementById("spread-count").value.trim();
        if (!k) return showToast("Enter K.", "error");
        try {
            const data = await api(`/api/news-spread?k=${encodeURIComponent(k)}`);
            setResult("spread-result", `Selected users: ${data.selected_users.join(", ")}`);
        } catch (error) {
            setResult("spread-result", error.message, "error");
        }
    });
}

function setupGraphPage() {
    const canvas = document.getElementById("graph-canvas");
    if (!canvas) return;
    const context = canvas.getContext("2d");
    const details = document.getElementById("node-details");
    const note = document.getElementById("graph-note");
    const status = document.getElementById("graph-status");
    const sizeInput = document.getElementById("node-size");

    let radius = Number(sizeInput?.value || 18);
    let labels = true;
    let nodes = [];
    let edges = [];
    let dragging = null;
    let dragOffset = {x: 0, y: 0};

    function canvasBox() {
        return canvas.getBoundingClientRect();
    }

    function arrangeNodes() {
        const box = canvasBox();
        if (!nodes.length) return;
        const cx = box.width / 2;
        const cy = box.height / 2;
        const maxPerRing = Math.max(8, Math.floor((Math.min(box.width, box.height) * Math.PI) / 80));
        let index = 0;
        let ring = 1;
        while (index < nodes.length) {
            const count = Math.min(maxPerRing * ring, nodes.length - index);
            const ringRadius = Math.min(box.width, box.height) * 0.12 * ring;
            for (let local = 0; local < count; local += 1) {
                const angle = -Math.PI / 2 + (2 * Math.PI * local) / count;
                nodes[index + local].x = cx + Math.cos(angle) * ringRadius;
                nodes[index + local].y = cy + Math.sin(angle) * ringRadius;
            }
            index += count;
            ring += 1;
        }
    }

    function draw() {
        const box = canvasBox();
        context.clearRect(0, 0, box.width, box.height);
        const byId = new Map(nodes.map((node) => [node.id, node]));
        context.strokeStyle = "#30363d";
        context.lineWidth = 1.3;
        edges.forEach(([first, second]) => {
            const a = byId.get(first);
            const b = byId.get(second);
            if (!a || !b) return;
            context.beginPath();
            context.moveTo(a.x, a.y);
            context.lineTo(b.x, b.y);
            context.stroke();
        });

        nodes.forEach((node) => {
            context.beginPath();
            context.fillStyle = "#1f6feb";
            context.strokeStyle = "#8cc8ff";
            context.lineWidth = 2;
            context.arc(node.x, node.y, radius, 0, Math.PI * 2);
            context.fill();
            context.stroke();
            if (labels) {
                context.fillStyle = "#e6edf3";
                context.font = "12px Segoe UI";
                context.textAlign = "center";
                context.fillText(node.id, node.x, node.y + 4);
            }
        });
    }

    function resize(resetPositions = false) {
        const box = canvasBox();
        const ratio = window.devicePixelRatio || 1;
        canvas.width = Math.max(320, Math.floor(box.width * ratio));
        canvas.height = Math.max(360, Math.floor(box.height * ratio));
        context.setTransform(ratio, 0, 0, ratio, 0, 0);
        if (resetPositions) arrangeNodes();
        draw();
    }

    function nodeAt(event) {
        const box = canvasBox();
        const x = event.clientX - box.left;
        const y = event.clientY - box.top;
        return {node: nodes.find((item) => Math.hypot(item.x - x, item.y - y) <= radius + 6), x, y};
    }

    canvas.addEventListener("pointerdown", (event) => {
        const hit = nodeAt(event);
        if (!hit.node) return;
        dragging = hit.node;
        dragOffset = {x: hit.node.x - hit.x, y: hit.node.y - hit.y};
        canvas.setPointerCapture(event.pointerId);
    });
    canvas.addEventListener("pointermove", (event) => {
        if (!dragging) return;
        const box = canvasBox();
        dragging.x = Math.max(radius, Math.min(box.width - radius, event.clientX - box.left + dragOffset.x));
        dragging.y = Math.max(radius, Math.min(box.height - radius, event.clientY - box.top + dragOffset.y));
        draw();
    });
    canvas.addEventListener("pointerup", () => { dragging = null; });
    canvas.addEventListener("pointercancel", () => { dragging = null; });
    canvas.addEventListener("click", (event) => {
        const hit = nodeAt(event);
        if (!hit.node || !details) return;
        clearElement(details);
        const title = document.createElement("strong");
        title.textContent = `${hit.node.name} (${hit.node.id})`;
        details.appendChild(title);
        const count = document.createElement("span");
        count.className = "detail-line";
        count.textContent = `${hit.node.friend_count} friend(s)`;
        details.appendChild(count);
        const friends = document.createElement("span");
        friends.className = "detail-line muted";
        friends.textContent = hit.node.friends.length ? `Friends: ${hit.node.friends.join(", ")}` : "No friends";
        details.appendChild(friends);
    });

    document.getElementById("graph-reset")?.addEventListener("click", () => resize(true));
    document.getElementById("graph-labels")?.addEventListener("click", (event) => {
        labels = !labels;
        event.currentTarget.textContent = labels ? "Hide labels" : "Show labels";
        draw();
    });
    sizeInput?.addEventListener("input", () => {
        radius = Number(sizeInput.value);
        draw();
    });
    window.addEventListener("resize", () => resize(true));

    api("/api/graph").then((data) => {
        nodes = data.nodes.map((node) => ({...node, x: 0, y: 0}));
        edges = data.edges;
        if (note) note.textContent = nodes.length
            ? `${nodes.length} users and ${edges.length} friendships. Drag nodes or click for details.`
            : "The network is empty.";
        if (status) status.textContent = `${nodes.length} nodes`;
        resize(true);
    }).catch((error) => {
        if (note) note.textContent = error.message;
        if (status) status.textContent = "Error";
        showToast(error.message, "error");
        resize(false);
    });
}

document.addEventListener("DOMContentLoaded", () => {
    setupMobileMenu();
    loadDashboard();
    document.getElementById("refresh-dashboard")?.addEventListener("click", loadDashboard);
    setupUsersPage();
    setupPathsPage();
    setupDistancesPage();
    setupManagementPage();
    setupGroupsPage();
    setupInsightsPage();
    setupGraphPage();
});
