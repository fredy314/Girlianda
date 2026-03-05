/*
 * Girlianda project
 * Copyright (c) 2026 Fedir Vilhota <fredy31415@gmail.com>
 * This software is released under the MIT License.
 * See the LICENSE file in the project root for full license information.
 */
#ifndef PAGES_HTML_H
#define PAGES_HTML_H

#include <Arduino.h>

const char HTML_CHRISTMAS_TREE_SVG[] PROGMEM = R"rawliteral(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100">
  <!-- Tree layers -->
  <polygon points="50,15 30,35 35,35 20,50 25,50 10,65 40,65 40,80 60,80 60,65 90,65 75,50 80,50 65,35 70,35" fill="#2d5016"/>
  <polygon points="50,18 32,36 36,36 23,49 27,49 13,63 40,63 40,78 60,78 60,63 87,63 73,49 77,49 64,36 68,36" fill="#3a7d2e"/>
  
  <!-- Trunk -->
  <rect x="45" y="65" width="10" height="15" fill="#5d4037"/>
  
  <!-- Decorations -->
  <circle cx="50" cy="28" r="2.5" fill="#ffd700"/>
  <circle cx="42" cy="38" r="2" fill="#ff1744"/>
  <circle cx="58" cy="42" r="2" fill="#2979ff"/>
  <circle cx="35" cy="52" r="2" fill="#ffd700"/>
  <circle cx="65" cy="55" r="2" fill="#ff1744"/>
  <circle cx="50" cy="58" r="2" fill="#2979ff"/>
  
  <!-- Star on top -->
  <polygon points="50,5 51.5,10 57,10 52.5,13.5 54,18.5 50,15 46,18.5 47.5,13.5 43,10 48.5,10" fill="#ffd700"/>
</svg>
)rawliteral";

const char HTML_HOME_BASE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="uk">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Гірлянда</title>
  <link rel="icon" type="image/svg+xml" href="/favicon.ico">
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { 
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
      background: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%);
      color: #fff;
      min-height: 100vh;
      padding: 20px;
    }
    .container { 
      max-width: 900px; 
      margin: 0 auto; 
      background: rgba(255,255,255,0.1);
      border-radius: 20px;
      padding: 30px;
      backdrop-filter: blur(10px);
      box-shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.37);
    }
    h1 { 
      text-align: center; 
      margin-bottom: 30px;
      font-size: 2.5em;
      text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
    }
    .garland-section {
      background: rgba(255,255,255,0.05);
      border-radius: 15px;
      padding: 25px;
      margin-bottom: 25px;
      border: 2px solid rgba(255,255,255,0.1);
    }
    .garland-section h2 {
      margin-bottom: 20px;
      font-size: 1.8em;
      color: #ffd700;
      text-shadow: 1px 1px 3px rgba(0,0,0,0.3);
    }
    .modes { 
      display: grid; 
      grid-template-columns: repeat(auto-fit, minmax(140px, 1fr)); 
      gap: 12px; 
      margin-bottom: 25px; 
    }
    .btn {
      padding: 15px 20px;
      border: none;
      border-radius: 12px;
      cursor: pointer;
      font-size: 16px;
      font-weight: 600;
      transition: all 0.3s ease;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      box-shadow: 0 4px 15px 0 rgba(102, 126, 234, 0.4);
      text-align: center;
    }
    .btn:hover { 
      transform: translateY(-2px); 
      box-shadow: 0 6px 20px 0 rgba(102, 126, 234, 0.6);
    }
    .btn:active { transform: translateY(0); }
    .btn.active { 
      background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
      box-shadow: 0 6px 20px 0 rgba(245, 87, 108, 0.6);
    }
    .control-group {
      margin-bottom: 20px;
    }
    .control-group label {
      display: block;
      margin-bottom: 8px;
      font-size: 16px;
      font-weight: 500;
    }
    .slider-container {
      display: flex;
      align-items: center;
      gap: 15px;
    }
    input[type="range"] {
      flex: 1;
      height: 8px;
      border-radius: 5px;
      background: rgba(255,255,255,0.2);
      outline: none;
      -webkit-appearance: none;
    }
    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 20px;
      height: 20px;
      border-radius: 50%;
      background: #ffd700;
      cursor: pointer;
      box-shadow: 0 0 10px rgba(255, 215, 0, 0.5);
    }
    input[type="range"]::-moz-range-thumb {
      width: 20px;
      height: 20px;
      border-radius: 50%;
      background: #ffd700;
      cursor: pointer;
      border: none;
      box-shadow: 0 0 10px rgba(255, 215, 0, 0.5);
    }
    .value-display {
      min-width: 50px;
      text-align: center;
      font-weight: 600;
      font-size: 18px;
      color: #ffd700;
    }
    .ota-link { 
      background: linear-gradient(135deg, #8e44ad 0%, #9b59b6 100%); 
      margin-top: 20px; 
      text-decoration: none; 
      display: block;
      text-align: center;
    }
    .ota-link:hover { 
      background: linear-gradient(135deg, #9b59b6 0%, #af7ac5 100%);
    }
    .download-link {
      background: linear-gradient(135deg, #16a085 0%, #1abc9c 100%);
      margin-top: 10px;
      text-decoration: none;
      display: block;
      text-align: center;
    }
    .download-link:hover {
      background: linear-gradient(135deg, #1abc9c 0%, #48c9b0 100%);
    }
    #status {
      text-align: center;
      margin-top: 20px;
      padding: 15px;
      background: rgba(255,255,255,0.1);
      border-radius: 10px;
      font-size: 14px;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🎄 Гірлянда</h1>
    
    <!-- Гірлянда A -->
    <div class="garland-section">
      <h2>Гірлянда A</h2>
      <div class="modes" id="modesA">
        <button class="btn" onclick="setMode('A', 0)">Постійне</button>
        <button class="btn" onclick="setMode('A', 1)">Почергове</button>
        <button class="btn" onclick="setMode('A', 2)">Дихання</button>
        <button class="btn" onclick="setMode('A', 3)">Хаос</button>
        <button class="btn" onclick="setMode('A', 4)">Свічка</button>
      </div>
      <div class="control-group">
        <label>Швидкість анімації</label>
        <div class="slider-container">
          <input type="range" id="speedA" min="1" max="100" value="30" oninput="setSpeed('A', this.value)">
          <span class="value-display" id="speedValueA">30</span>
        </div>
      </div>
      <div class="control-group">
        <label>Яскравість</label>
        <div class="slider-container">
          <input type="range" id="brightnessA" min="0" max="255" value="255" oninput="setBrightness('A', this.value)">
          <span class="value-display" id="brightnessValueA">255</span>
        </div>
      </div>
    </div>

    <!-- Гірлянда B -->
    <div class="garland-section">
      <h2>Гірлянда B</h2>
      <div class="modes" id="modesB">
        <button class="btn" onclick="setMode('B', 0)">Постійне</button>
        <button class="btn" onclick="setMode('B', 1)">Почергове</button>
        <button class="btn" onclick="setMode('B', 2)">Дихання</button>
        <button class="btn" onclick="setMode('B', 3)">Хаос</button>
        <button class="btn" onclick="setMode('B', 4)">Свічка</button>
      </div>
      <div class="control-group">
        <label>Швидкість анімації</label>
        <div class="slider-container">
          <input type="range" id="speedB" min="1" max="100" value="30" oninput="setSpeed('B', this.value)">
          <span class="value-display" id="speedValueB">30</span>
        </div>
      </div>
      <div class="control-group">
        <label>Яскравість</label>
        <div class="slider-container">
          <input type="range" id="brightnessB" min="0" max="255" value="255" oninput="setBrightness('B', this.value)">
          <span class="value-display" id="brightnessValueB">255</span>
        </div>
      </div>
    </div>
    
    <div id="status">Підключення...</div>
)rawliteral";

const char HTML_SCRIPTS_API[] PROGMEM = R"rawliteral(
<a href="/update" class="btn ota-link" target="_blank">🔄 Оновити прошивку</a>
  </div>
<script>
function setMode(garland, mode) {
  fetch(`/api/set?garland=${garland}&mode=${mode}`)
    .then(r => r.json())
    .then(data => {
      updateStatus();
      console.log('Mode set:', data);
    });
}

function setSpeed(garland, speed) {
  document.getElementById(`speedValue${garland}`).textContent = speed;
  fetch(`/api/set?garland=${garland}&speed=${speed}`)
    .then(r => r.json())
    .then(data => console.log('Speed set:', data));
}

function setBrightness(garland, brightness) {
  document.getElementById(`brightnessValue${garland}`).textContent = brightness;
  fetch(`/api/set?garland=${garland}&brightness=${brightness}`)
    .then(r => r.json())
    .then(data => console.log('Brightness set:', data));
}

function updateStatus() {
  fetch('/api/status')
    .then(r => r.json())
    .then(data => {
      // Update Garland A
      document.getElementById('speedA').value = data.garlandA.speed;
      document.getElementById('speedValueA').textContent = data.garlandA.speed;
      document.getElementById('brightnessA').value = data.garlandA.brightness;
      document.getElementById('brightnessValueA').textContent = data.garlandA.brightness;
      
      // Update Garland B
      document.getElementById('speedB').value = data.garlandB.speed;
      document.getElementById('speedValueB').textContent = data.garlandB.speed;
      document.getElementById('brightnessB').value = data.garlandB.brightness;
      document.getElementById('brightnessValueB').textContent = data.garlandB.brightness;
      
      // Highlight active modes
      document.querySelectorAll('#modesA .btn').forEach((btn, idx) => {
        btn.classList.toggle('active', idx === data.garlandA.mode);
      });
      document.querySelectorAll('#modesB .btn').forEach((btn, idx) => {
        btn.classList.toggle('active', idx === data.garlandB.mode);
      });
      
      document.getElementById('status').textContent = `✓ Підключено | IP: ${data.ip}`;
    })
    .catch(err => {
      document.getElementById('status').textContent = '✗ Помилка підключення';
      console.error(err);
    });
}

// Update status every 2 seconds
setInterval(updateStatus, 2000);
updateStatus();
</script>
</body>
</html>
)rawliteral";

#endif
