// @dependency-start
// responsibility Provides the runnable browser GUI prototype for the editor core.
// upstream implementation ../include/editor_proto/workspace_registry.hpp workspace and scroll API
// upstream implementation ../CMakeLists.txt builds and tests this executable
// downstream design README.md documents repository build entrypoints
// @dependency-end
#include "editor_proto/workspace_registry.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

namespace {

constexpr std::uint16_t kDefaultPort = 8765;

const char* action_name(editor_proto::DuplicateAction action) {
  switch (action) {
    case editor_proto::DuplicateAction::FocusExisting:
      return "focus-existing";
    case editor_proto::DuplicateAction::OpenNewView:
      return "open-new-view";
    case editor_proto::DuplicateAction::RegisterAlias:
      return "register-alias";
  }
  return "unknown";
}

std::string json_escape(std::string_view value) {
  std::string out;
  out.reserve(value.size() + 8);
  for (const char ch : value) {
    if (ch == '"' || ch == '\\') {
      out.push_back('\\');
      out.push_back(ch);
    } else if (ch == '\n') {
      out += "\\n";
    } else {
      out.push_back(ch);
    }
  }
  return out;
}

std::string actions_json(const std::vector<editor_proto::DuplicateAction>& actions) {
  std::ostringstream out;
  out << "[";
  for (std::size_t i = 0; i < actions.size(); ++i) {
    if (i > 0) out << ",";
    out << "\"" << action_name(actions[i]) << "\"";
  }
  out << "]";
  return out.str();
}

std::string notice_json(const editor_proto::DuplicateNotice& notice) {
  std::ostringstream out;
  out << "{"
      << "\"status\":\"" << (notice.is_duplicate ? "duplicate" : "new") << "\","
      << "\"path\":\"" << json_escape(notice.canonical_path) << "\","
      << "\"actions\":" << actions_json(notice.actions) << "}";
  return out.str();
}

std::string demo_json() {
  editor_proto::WorkspaceRegistry registry;
  editor_proto::ScrollController scroll;

  const auto file_first = registry.open_file("/Repo/src/main.cpp");
  const auto file_second = registry.open_file("\\repo\\src\\main.cpp");
  const auto root_first = registry.add_directory_root("/Work/ProjectA/");
  const auto root_second = registry.add_directory_root("/work/projecta");
  const auto pan = scroll.middle_button_pan(100.0, -10.0);

  std::ostringstream out;
  out << "{"
      << "\"fileFirst\":" << notice_json(file_first) << ","
      << "\"fileSecond\":" << notice_json(file_second) << ","
      << "\"rootFirst\":" << notice_json(root_first) << ","
      << "\"rootSecond\":" << notice_json(root_second) << ","
      << "\"scroll\":{\"offset\":" << pan.new_offset << ",\"velocity\":" << pan.velocity << "}"
      << "}";
  return out.str();
}

const char kIndexHtml[] = R"HTML(<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Editor Make Prototype</title>
  <style>
    :root {
      color-scheme: dark;
      --bg: #111315;
      --panel: #191d21;
      --panel-2: #20262b;
      --line: #343b42;
      --text: #e6edf3;
      --muted: #9aa7b2;
      --accent: #4f8cff;
      --ok: #37a66b;
      --warn: #d8892c;
      --danger: #e05d5d;
    }
    * { box-sizing: border-box; }
    html, body { height: 100%; margin: 0; }
    body {
      background: var(--bg);
      color: var(--text);
      font: 13px/1.45 system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
      overflow: hidden;
    }
    button, input, textarea {
      font: inherit;
    }
    .app {
      display: grid;
      grid-template-rows: 42px minmax(0, 1fr) 156px 24px;
      height: 100%;
      min-width: 760px;
    }
    .topbar {
      display: grid;
      grid-template-columns: 220px 1fr auto;
      align-items: center;
      gap: 10px;
      border-bottom: 1px solid var(--line);
      background: #15191d;
      padding: 0 10px;
    }
    .brand {
      font-weight: 700;
      letter-spacing: 0;
    }
    .toolbar {
      display: flex;
      align-items: center;
      gap: 6px;
      min-width: 0;
    }
    .toolbar button, .notice button, .tab, .status-chip {
      border: 1px solid var(--line);
      background: var(--panel-2);
      color: var(--text);
      min-height: 28px;
      border-radius: 6px;
      padding: 4px 9px;
    }
    .toolbar button:hover, .notice button:hover, .tab.active {
      border-color: var(--accent);
      background: #253142;
    }
    .workspace-name {
      color: var(--muted);
      text-align: right;
      white-space: nowrap;
    }
    .main {
      display: grid;
      grid-template-columns: 250px minmax(0, 1fr) 310px;
      min-height: 0;
    }
    .sidebar, .inspector, .terminal {
      background: var(--panel);
      border-color: var(--line);
    }
    .sidebar {
      border-right: 1px solid var(--line);
      display: grid;
      grid-template-rows: 39px minmax(0, 1fr);
      min-width: 0;
    }
    .search {
      padding: 7px;
      border-bottom: 1px solid var(--line);
    }
    .search input {
      width: 100%;
      border: 1px solid var(--line);
      border-radius: 6px;
      color: var(--text);
      background: #11161a;
      min-height: 26px;
      padding: 3px 8px;
    }
    .tree {
      overflow: auto;
      padding: 8px;
    }
    .tree-row {
      display: grid;
      grid-template-columns: 18px minmax(0, 1fr);
      align-items: center;
      min-height: 26px;
      border-radius: 5px;
      padding: 1px 6px;
    }
    button.tree-row {
      width: 100%;
      border: 0;
      color: inherit;
      background: transparent;
      text-align: left;
    }
    button.tree-row:hover {
      background: #202b36;
    }
    .tree-row.active {
      background: #253142;
      color: #ffffff;
    }
    .tree-row span:last-child {
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
    }
    .editor {
      display: grid;
      grid-template-rows: 38px minmax(0, 1fr);
      min-width: 0;
      background: #0f1215;
    }
    .tabs {
      display: flex;
      gap: 4px;
      align-items: end;
      padding: 6px 8px 0;
      border-bottom: 1px solid var(--line);
      overflow-x: auto;
    }
    .tab {
      min-width: 110px;
      text-align: left;
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
    }
    textarea {
      width: 100%;
      height: 100%;
      resize: none;
      border: 0;
      outline: 0;
      color: #d9e2ec;
      background: #0f1215;
      padding: 16px 18px;
      font: 14px/1.55 "Cascadia Code", "Consolas", monospace;
      tab-size: 2;
    }
    .inspector {
      border-left: 1px solid var(--line);
      display: grid;
      grid-template-rows: 39px minmax(0, 1fr);
      min-width: 0;
    }
    .panel-title {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 8px;
      min-height: 39px;
      border-bottom: 1px solid var(--line);
      padding: 0 10px;
      font-weight: 700;
    }
    .notice {
      padding: 10px;
      display: grid;
      gap: 10px;
      align-content: start;
      overflow: auto;
    }
    .notice-box {
      border: 1px solid var(--line);
      border-left: 3px solid var(--warn);
      border-radius: 6px;
      padding: 10px;
      background: #171b1f;
    }
    .notice-box.ok { border-left-color: var(--ok); }
    .notice-box strong { display: block; margin-bottom: 4px; }
    .actions {
      display: flex;
      flex-wrap: wrap;
      gap: 6px;
      margin-top: 8px;
    }
    .terminal {
      border-top: 1px solid var(--line);
      display: grid;
      grid-template-rows: 30px minmax(0, 1fr);
      min-height: 0;
    }
    .terminal pre {
      margin: 0;
      padding: 8px 12px;
      overflow: auto;
      color: #b7c7d9;
      font: 13px/1.45 "Cascadia Code", "Consolas", monospace;
      white-space: pre-wrap;
    }
    .status {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 12px;
      border-top: 1px solid var(--line);
      color: var(--muted);
      background: #15191d;
      padding: 0 10px;
      font-size: 12px;
    }
    .status-chip {
      min-height: 18px;
      padding: 0 7px;
      color: var(--ok);
      font-size: 12px;
    }
    @media (max-width: 920px) {
      .main { grid-template-columns: 210px minmax(0, 1fr); }
      .inspector { display: none; }
      .app { min-width: 620px; }
    }
  </style>
</head>
<body>
  <div class="app">
    <header class="topbar">
      <div class="brand">Editor Make</div>
      <nav class="toolbar" aria-label="toolbar">
        <button id="openFile" title="Open the sample file">Open</button>
        <button id="addRoot" title="Add the sample workspace root">Root</button>
        <button id="splitView" title="Create a second view">Split</button>
        <button id="saveFile" title="Save the active buffer">Save</button>
        <button id="runDemo" title="Reload core state">Run</button>
      </nav>
      <div class="workspace-name">workspace /Work/ProjectA</div>
    </header>
    <main class="main">
      <aside class="sidebar">
        <div class="search"><input id="filter" value="" placeholder="Filter"></div>
        <div id="tree" class="tree"></div>
      </aside>
      <section class="editor">
        <div id="tabs" class="tabs"></div>
        <textarea id="buffer" spellcheck="false"></textarea>
      </section>
      <aside class="inspector">
        <div class="panel-title"><span>Duplicate Notice</span><span class="status-chip" id="coreState">core</span></div>
        <div id="notice" class="notice"></div>
      </aside>
    </main>
    <section class="terminal">
      <div class="panel-title"><span>Shell</span><span id="scrollState">offset 0</span></div>
      <pre id="terminalLog"></pre>
    </section>
    <footer class="status">
      <span id="statusText">ready</span>
      <span id="cursorText">Ln 1, Col 1</span>
    </footer>
  </div>
  <script>
    const files = [
      {
        path: '/repo/src/main.cpp',
        label: 'main.cpp',
        content: '#include <iostream>\\n\\nint main() {\\n  std::cout << \"Editor Make\" << std::endl;\\n  return 0;\\n}\\n'
      },
      {
        path: '/repo/src/editor_proto_cli.cpp',
        label: 'editor_proto_cli.cpp',
        content: '// CLI prototype surface\\n// GUI prototype is connected through the same duplicate core.\\n'
      },
      {
        path: '/repo/notes/prototype_execution_plan_ja.md',
        label: 'prototype_execution_plan_ja.md',
        content: '# Prototype Plan\\n\\n- Window shell\\n- Directory panel\\n- Editor pane\\n- Shell pane\\n'
      }
    ];
    const roots = ['/work/projecta'];
    const openTabs = [];
    let activePath = '';
    let splitCount = 1;
    let dirty = false;

    const tree = document.getElementById('tree');
    const tabs = document.getElementById('tabs');
    const buffer = document.getElementById('buffer');
    const notice = document.getElementById('notice');
    const terminalLog = document.getElementById('terminalLog');
    const statusText = document.getElementById('statusText');
    const cursorText = document.getElementById('cursorText');
    const coreState = document.getElementById('coreState');
    const scrollState = document.getElementById('scrollState');

    function log(line) {
      const now = new Date().toLocaleTimeString();
      terminalLog.textContent += `[${now}] ${line}\\n`;
      terminalLog.scrollTop = terminalLog.scrollHeight;
    }

    function setStatus(text) {
      statusText.textContent = text;
    }

    function renderTree() {
      const query = document.getElementById('filter').value.toLowerCase();
      tree.innerHTML = '';
      roots.forEach(root => {
        const rootRow = document.createElement('div');
        rootRow.className = 'tree-row active';
        rootRow.innerHTML = '<span>/</span><span>' + root + '</span>';
        tree.appendChild(rootRow);
      });
      files.filter(f => f.path.includes(query) || f.label.toLowerCase().includes(query)).forEach(file => {
        const row = document.createElement('button');
        row.className = 'tree-row';
        row.type = 'button';
        row.innerHTML = '<span>F</span><span>' + file.label + '</span>';
        row.onclick = () => openFile(file.path);
        tree.appendChild(row);
      });
    }

    function renderTabs() {
      tabs.innerHTML = '';
      openTabs.forEach(path => {
        const file = files.find(f => f.path === path);
        const tab = document.createElement('button');
        tab.className = 'tab' + (path === activePath ? ' active' : '');
        tab.textContent = (file ? file.label : path) + (dirty && path === activePath ? ' *' : '');
        tab.onclick = () => focusFile(path);
        tabs.appendChild(tab);
      });
    }

    function focusFile(path) {
      const file = files.find(f => f.path === path);
      if (!file) return;
      activePath = path;
      buffer.value = file.content;
      dirty = false;
      renderTabs();
      setStatus('focused ' + path);
      updateCursor();
    }

    function showNotice(kind, data) {
      const title = data.status === 'duplicate' ? `${kind} duplicate` : `${kind} opened`;
      const cls = data.status === 'duplicate' ? 'notice-box' : 'notice-box ok';
      const actions = data.actions || [];
      notice.innerHTML = `
        <div class="${cls}">
          <strong>${title}</strong>
          <div>${data.path}</div>
          <div class="actions"></div>
        </div>`;
      const actionSlot = notice.querySelector('.actions');
      if (actions.length === 0) {
        const quiet = document.createElement('span');
        quiet.textContent = 'no conflict';
        quiet.style.color = 'var(--muted)';
        actionSlot.appendChild(quiet);
      }
      actions.forEach(action => {
        const button = document.createElement('button');
        button.textContent = action;
        button.onclick = () => {
          setStatus(action + ' applied');
          log(`${kind}: ${action} -> ${data.path}`);
        };
        actionSlot.appendChild(button);
      });
    }

    function openFile(path) {
      const duplicate = openTabs.includes(path);
      if (!duplicate) openTabs.push(path);
      focusFile(path);
      showNotice('file', {
        status: duplicate ? 'duplicate' : 'new',
        path,
        actions: duplicate ? ['focus-existing', 'open-new-view'] : []
      });
      log(`open file ${path} (${duplicate ? 'duplicate' : 'new'})`);
    }

    function addRoot() {
      const path = '/work/projecta';
      const duplicate = roots.includes(path);
      showNotice('root', {
        status: duplicate ? 'duplicate' : 'new',
        path,
        actions: duplicate ? ['focus-existing', 'register-alias'] : []
      });
      log(`add root ${path} (${duplicate ? 'duplicate' : 'new'})`);
    }

    async function runCoreDemo() {
      const response = await fetch('/api/demo');
      const demo = await response.json();
      coreState.textContent = 'core ok';
      showNotice('file', demo.fileSecond);
      scrollState.textContent = `offset ${demo.scroll.offset}`;
      log(`core duplicate file -> ${demo.fileSecond.actions.join('|')}`);
      log(`core duplicate root -> ${demo.rootSecond.actions.join('|')}`);
      log(`core pan offset=${demo.scroll.offset} velocity=${demo.scroll.velocity}`);
    }

    function updateCursor() {
      const value = buffer.value.slice(0, buffer.selectionStart);
      const lines = value.split('\\n');
      cursorText.textContent = `Ln ${lines.length}, Col ${lines[lines.length - 1].length + 1}`;
    }

    document.getElementById('openFile').onclick = () => openFile('/repo/src/main.cpp');
    document.getElementById('addRoot').onclick = addRoot;
    document.getElementById('splitView').onclick = () => {
      splitCount += 1;
      setStatus(`split view ${splitCount}`);
      log(`split view ${splitCount}`);
    };
    document.getElementById('saveFile').onclick = () => {
      const file = files.find(f => f.path === activePath);
      if (file) file.content = buffer.value;
      dirty = false;
      renderTabs();
      setStatus('saved ' + activePath);
      log('save ' + activePath);
    };
    document.getElementById('runDemo').onclick = runCoreDemo;
    document.getElementById('filter').oninput = renderTree;
    buffer.addEventListener('input', () => {
      dirty = true;
      renderTabs();
      setStatus('editing ' + activePath);
      updateCursor();
    });
    buffer.addEventListener('keyup', updateCursor);
    buffer.addEventListener('click', updateCursor);
    buffer.addEventListener('wheel', event => {
      scrollState.textContent = `wheel ${Math.trunc(event.deltaY)}`;
    }, { passive: true });

    renderTree();
    openFile('/repo/src/main.cpp');
    runCoreDemo();
  </script>
</body>
</html>)HTML";

bool send_all(int client, std::string_view body) {
  std::size_t sent = 0;
  while (sent < body.size()) {
    const auto result = ::send(client, body.data() + sent, body.size() - sent, 0);
    if (result <= 0) return false;
    sent += static_cast<std::size_t>(result);
  }
  return true;
}

void send_response(int client, int status, std::string_view content_type, std::string_view body) {
  const std::string status_text = status == 200 ? "OK" : "Not Found";
  std::ostringstream headers;
  headers << "HTTP/1.1 " << status << " " << status_text << "\r\n"
          << "Content-Type: " << content_type << "\r\n"
          << "Content-Length: " << body.size() << "\r\n"
          << "Cache-Control: no-store\r\n"
          << "Connection: close\r\n\r\n";
  const std::string header_text = headers.str();
  (void)send_all(client, header_text);
  (void)send_all(client, body);
}

std::string request_path(std::string_view request) {
  const auto line_end = request.find("\r\n");
  const auto first_line = request.substr(0, line_end);
  const auto first_space = first_line.find(' ');
  if (first_space == std::string_view::npos) return "/";
  const auto second_space = first_line.find(' ', first_space + 1);
  auto path = first_line.substr(first_space + 1, second_space - first_space - 1);
  const auto query = path.find('?');
  if (query != std::string_view::npos) path = path.substr(0, query);
  return std::string(path);
}

int make_server_socket(std::uint16_t port) {
  const int server = ::socket(AF_INET, SOCK_STREAM, 0);
  if (server < 0) {
    throw std::runtime_error(std::string("socket failed: ") + std::strerror(errno));
  }

  int reuse = 1;
  (void)::setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  address.sin_port = htons(port);

  if (::bind(server, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
    const std::string message = std::string("bind failed: ") + std::strerror(errno);
    ::close(server);
    throw std::runtime_error(message);
  }
  if (::listen(server, 16) < 0) {
    const std::string message = std::string("listen failed: ") + std::strerror(errno);
    ::close(server);
    throw std::runtime_error(message);
  }
  return server;
}

void serve_forever(std::uint16_t port) {
  const int server = make_server_socket(port);
  std::cout << "Editor GUI prototype: http://127.0.0.1:" << port << "\n";
  std::cout << "Press Ctrl+C to stop.\n" << std::flush;

  while (true) {
    const int client = ::accept(server, nullptr, nullptr);
    if (client < 0) continue;

    char buffer[4096]{};
    const auto bytes = ::recv(client, buffer, sizeof(buffer) - 1, 0);
    const std::string request(buffer, bytes > 0 ? static_cast<std::size_t>(bytes) : 0);
    const std::string path = request_path(request);

    if (path == "/" || path == "/index.html") {
      send_response(client, 200, "text/html; charset=utf-8", kIndexHtml);
    } else if (path == "/api/demo") {
      send_response(client, 200, "application/json; charset=utf-8", demo_json());
    } else if (path == "/health") {
      send_response(client, 200, "text/plain; charset=utf-8", "ok\n");
    } else {
      send_response(client, 404, "text/plain; charset=utf-8", "not found\n");
    }

    ::close(client);
  }
}

void print_usage() {
  std::cout << "usage: editor_gui_proto [--port PORT|--smoke]\n";
}

}  // namespace

int main(int argc, char** argv) {
  std::uint16_t port = kDefaultPort;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--help") {
      print_usage();
      return 0;
    }
    if (arg == "--smoke") {
      std::cout << "editor_gui_proto smoke ok\n";
      std::cout << demo_json() << "\n";
      return 0;
    }
    if (arg == "--port" && i + 1 < argc) {
      int value = 0;
      try {
        value = std::stoi(argv[++i]);
      } catch (const std::exception& exc) {
        std::cerr << "invalid port: " << exc.what() << "\n";
        return 1;
      }
      if (value < 1 || value > 65535) {
        std::cerr << "port out of range\n";
        return 1;
      }
      port = static_cast<std::uint16_t>(value);
      continue;
    }
    print_usage();
    return 1;
  }

  try {
    serve_forever(port);
  } catch (const std::exception& exc) {
    std::cerr << exc.what() << "\n";
    return 1;
  }
  return 0;
}
