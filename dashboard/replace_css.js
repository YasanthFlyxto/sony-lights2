const fs = require('fs');
const path = require('path');

const css = `  <style>
    :root {
      --bg:       #050508;
      --card:     rgba(20, 20, 32, 0.5);
      --border:   rgba(255,255,255,0.06);
      --accent:   #7c3aed;
      --accent2:  #a78bfa;
      --glow:     rgba(124,58,237,0.4);
      --online:   #22c55e;
      --offline:  #475569;
      --text:     #f8f8f8;
      --text2:    #9ca3af;
      --r:        16px;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    
    @keyframes bgDrift {
      0% { background-position: 0% 50%; }
      50% { background-position: 100% 50%; }
      100% { background-position: 0% 50%; }
    }

    body {
      font-family: 'Inter', sans-serif;
      background: var(--bg);
      background-image: radial-gradient(circle at 15% 50%, rgba(124,58,237,0.08), transparent 40%),
                        radial-gradient(circle at 85% 30%, rgba(34,197,94,0.05), transparent 40%);
      background-size: 200% 200%;
      animation: bgDrift 15s ease infinite;
      color: var(--text);
      min-height: 100vh;
      overflow-x: hidden;
    }

    /* ── Header ── */
    header {
      display: flex; align-items: center; justify-content: space-between;
      padding: 16px 28px;
      background: rgba(10, 10, 16, 0.7);
      backdrop-filter: blur(16px);
      -webkit-backdrop-filter: blur(16px);
      border-bottom: 1px solid var(--border);
      position: sticky; top: 0; z-index: 100;
      box-shadow: 0 4px 30px rgba(0,0,0,0.5);
    }
    .logo { display: flex; align-items: center; gap: 12px; }
    .logo-dot {
      width: 12px; height: 12px; border-radius: 50%;
      background: var(--accent);
      box-shadow: 0 0 16px var(--accent);
      animation: pulse 2s infinite;
    }
    @keyframes pulse { 0%,100%{opacity:1; transform:scale(1);} 50%{opacity:0.6; transform:scale(0.85);} }
    .logo h1 { font-size: 20px; font-weight: 700; letter-spacing: -0.5px; }
    .logo span { color: var(--accent2); font-weight: 400; }
    #online-count { font-size: 14px; color: var(--text2); font-weight: 500; }
    #online-count b { color: var(--online); }

    .btn {
      padding: 9px 18px; border-radius: 10px; border: none;
      font-family: 'Inter', sans-serif; font-size: 13px; font-weight: 600;
      cursor: pointer; transition: all 0.2s cubic-bezier(0.175, 0.885, 0.32, 1.275);
    }
    .btn:active { transform: scale(0.95); }
    .btn-primary {
      background: linear-gradient(135deg, var(--accent), #6d28d9); color: #fff;
      box-shadow: 0 4px 15px rgba(124,58,237,0.2);
    }
    .btn-primary:hover { box-shadow: 0 6px 20px var(--glow); transform: translateY(-1px); }
    .btn-outline {
      background: rgba(255,255,255,0.03); color: var(--text);
      border: 1px solid var(--border);
    }
    .btn-outline:hover { border-color: var(--accent2); color: #fff; background: rgba(255,255,255,0.06); }
    .btn-sm { padding: 6px 12px; font-size: 12px; border-radius: 8px; }

    /* ── Main ── */
    main { padding: 24px 32px; max-width: 1400px; margin: 0 auto; }

    /* Animations */
    @keyframes fadeUp {
      from { opacity: 0; transform: translateY(20px); }
      to { opacity: 1; transform: translateY(0); }
    }

    /* ── Master Control ── */
    .master-card {
      background: var(--card);
      backdrop-filter: blur(24px);
      -webkit-backdrop-filter: blur(24px);
      border: 1px solid var(--border);
      border-radius: var(--r);
      padding: 24px 28px;
      margin-bottom: 32px;
      box-shadow: 0 10px 40px rgba(0,0,0,0.3);
      animation: fadeUp 0.6s ease-out;
    }
    .master-card h2 {
      font-size: 12px; font-weight: 600; letter-spacing: 1.5px;
      color: var(--text2); text-transform: uppercase; margin-bottom: 20px;
    }
    .master-controls {
      display: flex; align-items: center; gap: 24px; flex-wrap: wrap;
    }
    .control-group { display: flex; flex-direction: column; gap: 8px; }
    .control-group label { font-size: 12px; color: var(--text2); font-weight: 500; }

    input[type="color"] {
      width: 54px; height: 42px; border: 1px solid var(--border);
      border-radius: 10px; padding: 3px; background: rgba(0,0,0,0.2); cursor: pointer;
      transition: border 0.2s;
    }
    input[type="color"]:hover { border-color: var(--accent2); }

    input[type="range"] {
      -webkit-appearance: none; appearance: none;
      height: 6px; border-radius: 6px; background: rgba(255,255,255,0.05);
      outline: none; cursor: pointer; width: 140px;
      border: 1px solid rgba(255,255,255,0.05);
    }
    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none; width: 18px; height: 18px;
      border-radius: 50%; background: #fff; cursor: pointer;
      box-shadow: 0 2px 8px rgba(0,0,0,0.5), 0 0 0 4px var(--accent);
      transition: transform 0.1s;
    }
    input[type="range"]::-webkit-slider-thumb:hover { transform: scale(1.15); }

    .fx-buttons { display: flex; gap: 8px; flex-wrap: wrap; }
    .fx-btn {
      padding: 8px 14px; border-radius: 8px; font-size: 12px; font-weight: 500;
      border: 1px solid var(--border); background: rgba(255,255,255,0.03);
      color: var(--text2); cursor: pointer; transition: all 0.2s;
    }
    .fx-btn:hover { border-color: var(--accent2); color: var(--text); background: rgba(255,255,255,0.08); }
    .fx-btn.active {
      background: var(--accent); border-color: var(--accent);
      color: #fff; box-shadow: 0 4px 15px var(--glow);
    }

    #sync-btn {
      padding: 12px 28px; font-size: 14px; font-weight: 700;
      background: linear-gradient(135deg, var(--accent), #6d28d9);
      color: #fff; border: none; border-radius: 12px; cursor: pointer;
      box-shadow: 0 6px 20px var(--glow); transition: all 0.2s cubic-bezier(0.175, 0.885, 0.32, 1.275);
      letter-spacing: 0.5px;
    }
    #sync-btn:hover { transform: translateY(-2px) scale(1.02); box-shadow: 0 8px 30px var(--glow); }
    #sync-btn:active { transform: translateY(0) scale(0.98); }

    .divider { width: 1px; height: 48px; background: var(--border); }

    /* ── Lamp Grid ── */
    .section-header {
      display: flex; align-items: center; justify-content: space-between;
      margin-bottom: 20px;
    }
    .section-header h2 {
      font-size: 12px; font-weight: 600; letter-spacing: 1.5px;
      color: var(--text2); text-transform: uppercase;
    }
    .lamp-grid {
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(220px, 1fr));
      gap: 18px;
    }
    .lamp-card {
      background: var(--card); 
      backdrop-filter: blur(16px); -webkit-backdrop-filter: blur(16px);
      border: 1px solid var(--border);
      border-radius: var(--r); padding: 18px;
      transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
      animation: fadeUp 0.6s ease-out backwards;
    }
    .lamp-card.online { border-color: rgba(34,197,94,0.15); box-shadow: 0 4px 20px rgba(0,0,0,0.2); }
    .lamp-card.online:hover {
      border-color: rgba(34,197,94,0.3);
      box-shadow: 0 12px 30px rgba(0,0,0,0.4), 0 0 20px rgba(34,197,94,0.05);
      transform: translateY(-4px);
    }
    .lamp-card.offline { opacity: 0.4; filter: grayscale(80%); }
    .lamp-card.offline:hover { opacity: 0.6; }

    .card-header {
      display: flex; align-items: center; justify-content: space-between;
      margin-bottom: 14px;
    }
    .card-title { display: flex; align-items: center; gap: 10px; }
    .status-dot {
      width: 10px; height: 10px; border-radius: 50%;
      background: var(--offline); transition: background 0.3s, box-shadow 0.3s;
    }
    .status-dot.online {
      background: var(--online);
      box-shadow: 0 0 8px var(--online);
    }
    .card-title span { font-size: 15px; font-weight: 600; letter-spacing: -0.3px; }
    .card-ip { font-size: 11px; color: var(--text2); font-family: monospace; }

    .color-swatch {
      width: 100%; height: 65px; border-radius: 10px;
      background: rgb(255,80,0); margin-bottom: 16px;
      border: 1px solid rgba(255,255,255,0.1); 
      box-shadow: inset 0 2px 10px rgba(0,0,0,0.3);
      transition: transform 0.2s, filter 0.2s;
    }
    .color-swatch:hover { filter: brightness(1.1); transform: scale(1.02); }

    .card-row {
      display: flex; align-items: center; justify-content: space-between;
      margin-bottom: 12px;
    }
    .card-label { font-size: 12px; color: var(--text2); font-weight: 500; }

    .card-br-slider {
      width: 100%; height: 4px; border-radius: 4px;
      background: rgba(255,255,255,0.05); outline: none; cursor: pointer;
      -webkit-appearance: none; appearance: none;
      margin-bottom: 14px; border: 1px solid rgba(255,255,255,0.02);
    }
    .card-br-slider::-webkit-slider-thumb {
      -webkit-appearance: none; width: 14px; height: 14px;
      border-radius: 50%; background: #fff; cursor: pointer;
      box-shadow: 0 0 10px var(--glow); transition: transform 0.1s;
    }
    .card-br-slider::-webkit-slider-thumb:hover { transform: scale(1.2); }

    .power-toggle {
      width: 44px; height: 24px; border-radius: 12px;
      background: rgba(255,255,255,0.1); border: 1px solid rgba(255,255,255,0.05); cursor: pointer;
      position: relative; transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
      box-shadow: inset 0 2px 4px rgba(0,0,0,0.2);
    }
    .power-toggle.on { background: var(--online); border-color: var(--online); box-shadow: 0 0 15px rgba(34,197,94,0.3); }
    .power-toggle::after {
      content: ''; position: absolute; top: 2px; left: 2px;
      width: 18px; height: 18px; border-radius: 50%;
      background: #fff; transition: left 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
      box-shadow: 0 2px 5px rgba(0,0,0,0.3);
    }
    .power-toggle.on::after { left: 22px; }

    .fx-badge {
      font-size: 11px; padding: 4px 10px; border-radius: 6px;
      background: rgba(124,58,237,0.15); color: var(--accent2);
      font-weight: 600; border: 1px solid rgba(124,58,237,0.2);
    }

    /* ── Settings Modal ── */
    .modal-overlay {
      display: none; position: fixed; inset: 0;
      background: rgba(0,0,0,0.8); backdrop-filter: blur(8px);
      z-index: 200; align-items: center; justify-content: center;
      opacity: 0; transition: opacity 0.3s;
    }
    .modal-overlay.open { display: flex; opacity: 1; }
    .modal {
      background: var(--card); border: 1px solid var(--border);
      border-radius: 24px; padding: 32px; width: 560px; max-width: 95vw;
      max-height: 90vh; overflow-y: auto; box-shadow: 0 20px 60px rgba(0,0,0,0.6);
      transform: scale(0.95); transition: transform 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
    }
    .modal-overlay.open .modal { transform: scale(1); }
    .modal h2 { font-size: 20px; font-weight: 700; margin-bottom: 8px; }
    .modal p { font-size: 14px; color: var(--text2); margin-bottom: 24px; line-height: 1.5; }
    .auto-fill {
      display: flex; gap: 10px; align-items: center; margin-bottom: 24px;
      padding: 16px; background: rgba(0,0,0,0.2); border-radius: 12px;
      border: 1px solid var(--border);
    }
    .auto-fill input[type="text"] {
      flex: 1; background: rgba(0,0,0,0.3); border: 1px solid var(--border);
      border-radius: 8px; padding: 9px 12px; color: var(--text);
      font-family: 'Inter', sans-serif; font-size: 14px; outline: none; transition: border 0.2s;
    }
    .auto-fill input:focus { border-color: var(--accent); }
    .ip-grid {
      display: grid; grid-template-columns: 1fr 1fr; gap: 14px;
      margin-bottom: 28px;
    }
    .ip-row { display: flex; flex-direction: column; gap: 6px; }
    .ip-row label { font-size: 12px; color: var(--text2); font-weight: 600; }
    .ip-row input {
      background: rgba(0,0,0,0.2); border: 1px solid var(--border);
      border-radius: 8px; padding: 9px 12px; color: var(--text);
      font-family: monospace; font-size: 14px; outline: none; transition: border 0.2s;
    }
    .ip-row input:focus { border-color: var(--accent); box-shadow: 0 0 0 2px rgba(124,58,237,0.2); }
    .modal-footer { display: flex; gap: 12px; justify-content: flex-end; }

    /* ── Toast ── */
    #toast {
      position: fixed; bottom: 32px; right: 32px;
      background: var(--card); backdrop-filter: blur(16px);
      border: 1px solid var(--border);
      border-radius: 12px; padding: 14px 22px; font-size: 14px; font-weight: 500;
      box-shadow: 0 10px 40px rgba(0,0,0,0.5);
      transform: translateY(100px); opacity: 0;
      transition: all 0.4s cubic-bezier(0.175, 0.885, 0.32, 1.275); z-index: 300; pointer-events: none;
    }
    #toast.show { transform: translateY(0); opacity: 1; }
  </style>`;

const p = path.join(__dirname, 'index.html');
let content = fs.readFileSync(p, 'utf8');

const startIdx = content.indexOf('<style>');
const endIdx = content.indexOf('</style>') + 8;

if (startIdx !== -1 && endIdx !== -1) {
    content = content.substring(0, startIdx) + css + content.substring(endIdx);
    
    // Add staggered animation delay inline
    content = content.replace(/<div class="lamp-card" id="card-\${i}">/g, '<div class="lamp-card" id="card-${i}" style="animation-delay: ${i * 0.05}s">');

    fs.writeFileSync(p, content, 'utf8');
    console.log("Success");
} else {
    console.error("Could not find style block");
}
