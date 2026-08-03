function showToast(message, kind = "info") {
    const toast = document.getElementById("toast");
    if (!toast) return;
    toast.textContent = message;
    toast.dataset.kind = kind;
    toast.classList.add("visible");
    window.setTimeout(() => toast.classList.remove("visible"), 2600);
}

function showPreparedResult(formId, resultId, description) {
    const form = document.getElementById(formId);
    if (!form) return;

    form.addEventListener("submit", (event) => {
        event.preventDefault();
        const fields = [...form.querySelectorAll("input")];
        const missing = fields.some((field) => !field.value.trim());
        if (missing) {
            showToast("Please complete the required fields.", "error");
            return;
        }
        const result = document.getElementById(resultId);
        if (result) {
            result.textContent = description;
            result.classList.add("visible");
        }
        showToast("Interface action prepared.", "success");
    });
}

document.addEventListener("DOMContentLoaded", () => {
    const forms = [
        ["user-search-form", "user-search-result", "The user result card is ready for project data."],
        ["mutual-form", "mutual-result", "The mutual-friends result area is ready for project data."],
        ["path-form", "path-result", "The path visualization is ready for project data."],
        ["distance-form", "distance-result", "The distance table is ready for project data."],
        ["add-user-form", "add-user-result", "The add-user form passed interface validation."],
        ["edit-user-form", "edit-user-result", "The edit-user form passed interface validation."],
        ["remove-user-form", "remove-user-result", "The remove-user form passed interface validation."],
        ["add-friendship-form", "add-friendship-result", "The add-friendship form passed interface validation."],
        ["remove-friendship-form", "remove-friendship-result", "The remove-friendship form passed interface validation."]
    ];
    forms.forEach((item) => showPreparedResult(...item));

    const loadGroups = document.getElementById("load-groups");
    const groupsResult = document.getElementById("groups-result");
    if (loadGroups && groupsResult) {
        loadGroups.addEventListener("click", () => {
            groupsResult.innerHTML = '<div class="empty-state bordered">Connected-component cards will be inserted in this section.</div>';
            showToast("Group view refreshed.", "success");
        });
    }

    document.getElementById("refresh-user-table")?.addEventListener("click", () => {
        showToast("The user table layout is ready for data.", "success");
    });
});


function bindActionButton(buttonId, resultId, loadingText, readyText) {
    const button = document.getElementById(buttonId);
    const result = document.getElementById(resultId);
    if (!button || !result) return;
    button.addEventListener("click", () => {
        button.disabled = true;
        result.textContent = loadingText;
        result.classList.add("visible", "loading-result");
        window.setTimeout(() => {
            result.textContent = readyText;
            result.classList.remove("loading-result");
            button.disabled = false;
            showToast("Result layout prepared.", "success");
        }, 450);
    });
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

function setupGraphPreview() {
    const canvas = document.getElementById("graph-canvas");
    if (!canvas) return;
    const ctx = canvas.getContext("2d");
    const details = document.getElementById("node-details");
    const sizeInput = document.getElementById("node-size");
    let radius = Number(sizeInput?.value || 18);
    let labels = true;
    let nodes = [];
    const edges = [[0,1],[0,2],[1,3],[2,3],[2,4],[3,5],[4,5],[5,6]];

    function makeNodes(w, h) {
        return [
            {id:"A",x:w*.18,y:h*.45},{id:"B",x:w*.34,y:h*.22},{id:"C",x:w*.36,y:h*.68},
            {id:"D",x:w*.54,y:h*.40},{id:"E",x:w*.58,y:h*.76},{id:"F",x:w*.73,y:h*.50},{id:"G",x:w*.86,y:h*.28}
        ];
    }
    function draw() {
        const box = canvas.getBoundingClientRect();
        ctx.clearRect(0,0,box.width,box.height);
        ctx.strokeStyle = "#30363d"; ctx.lineWidth = 2;
        edges.forEach(([a,b]) => { ctx.beginPath(); ctx.moveTo(nodes[a].x,nodes[a].y); ctx.lineTo(nodes[b].x,nodes[b].y); ctx.stroke(); });
        nodes.forEach((n) => {
            ctx.beginPath(); ctx.fillStyle="#1f6feb"; ctx.strokeStyle="#8cc8ff"; ctx.lineWidth=2; ctx.arc(n.x,n.y,radius,0,Math.PI*2); ctx.fill(); ctx.stroke();
            if (labels) { ctx.fillStyle="#e6edf3"; ctx.font="12px Segoe UI"; ctx.textAlign="center"; ctx.fillText(n.id,n.x,n.y+4); }
        });
    }
    function resize() {
        const box = canvas.getBoundingClientRect();
        const ratio = window.devicePixelRatio || 1;
        canvas.width = Math.max(320, Math.floor(box.width*ratio)); canvas.height = Math.max(360, Math.floor(box.height*ratio));
        ctx.setTransform(ratio,0,0,ratio,0,0); nodes = makeNodes(box.width,box.height); draw();
    }
    canvas.addEventListener("click", (event) => {
        const box = canvas.getBoundingClientRect(); const x=event.clientX-box.left; const y=event.clientY-box.top;
        const n=nodes.find((item)=>Math.hypot(item.x-x,item.y-y)<=radius+6); if(!n||!details) return;
        details.innerHTML = `<strong>User ${n.id}</strong><span class="detail-line">User ID: ${n.id}</span><span class="detail-line muted">Friend and degree values will appear here.</span>`;
    });
    document.getElementById("graph-reset")?.addEventListener("click", resize);
    document.getElementById("graph-labels")?.addEventListener("click", (event) => { labels=!labels; event.currentTarget.textContent=labels?"Hide labels":"Show labels"; draw(); });
    sizeInput?.addEventListener("input", ()=>{ radius=Number(sizeInput.value); draw(); });
    window.addEventListener("resize", resize); resize();
}

document.addEventListener("DOMContentLoaded", () => {
    setupMobileMenu(); setupGraphPreview();
    bindActionButton("key-users-button","key-users-result","Preparing the ranking layout…","Key-user ranking cards are ready for project results.");
    bindActionButton("community-button","community-result","Preparing the community layout…","Community cards are ready for project results.");
    showPreparedResult("spread-form","spread-result","The initial-user selection area is ready for project results.");
});
