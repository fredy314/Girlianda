#include "PagesHandlers.h"
#include <WiFi.h>

PagesHandlers::PagesHandlers(Garland& garlandA, Garland& garlandB) 
    : _garlandA(garlandA), _garlandB(garlandB) {
}

void PagesHandlers::initPagesHandlers(AsyncWebServer& webServer) {
    setupHomePageHandler(webServer);
    setupApiStatusHandler(webServer);
    setupApiSetHandler(webServer);
}

void PagesHandlers::setupHomePageHandler(AsyncWebServer& webServer) {
    webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = R"rawliteral(
<!DOCTYPE html>
<html lang="uk">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Гірлянда</title>
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

    <a href="/update" class="btn ota-link" target="_blank">🔄 Оновити прошивку</a>
    
    <div id="status">Підключення...</div>
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
        request->send(200, "text/html", html);
    });
}

void PagesHandlers::setupApiStatusHandler(AsyncWebServer& webServer) {
    webServer.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request){
        String json = "{";
        json += "\"garlandA\":{";
        json += "\"mode\":" + String(_garlandA.getMode()) + ",";
        json += "\"speed\":" + String(_garlandA.getSpeed()) + ",";
        json += "\"brightness\":" + String(_garlandA.getBrightness());
        json += "},";
        json += "\"garlandB\":{";
        json += "\"mode\":" + String(_garlandB.getMode()) + ",";
        json += "\"speed\":" + String(_garlandB.getSpeed()) + ",";
        json += "\"brightness\":" + String(_garlandB.getBrightness());
        json += "},";
        json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
        json += "}";
        request->send(200, "application/json", json);
    });
}

void PagesHandlers::setupApiSetHandler(AsyncWebServer& webServer) {
    webServer.on("/api/set", HTTP_GET, [this](AsyncWebServerRequest *request){
        String garland = "";
        Garland* targetGarland = nullptr;
        
        // Визначаємо цільову гірлянду
        if (request->hasParam("garland")) {
            garland = request->getParam("garland")->value();
            if (garland == "A") {
                targetGarland = &_garlandA;
            } else if (garland == "B") {
                targetGarland = &_garlandB;
            }
        }
        
        if (targetGarland == nullptr) {
            request->send(400, "application/json", "{\"error\":\"Invalid garland parameter\"}");
            return;
        }
        
        // Обробка параметрів
        if (request->hasParam("mode")) {
            int mode = request->getParam("mode")->value().toInt();
            targetGarland->setMode(mode);
        }
        if (request->hasParam("speed")) {
            int speed = request->getParam("speed")->value().toInt();
            targetGarland->setSpeed(speed);
        }
        if (request->hasParam("brightness")) {
            int brightness = request->getParam("brightness")->value().toInt();
            targetGarland->setBrightness(brightness);
        }
        
        // Повертаємо поточний стан
        String json = "{";
        json += "\"garland\":\"" + garland + "\",";
        json += "\"mode\":" + String(targetGarland->getMode()) + ",";
        json += "\"speed\":" + String(targetGarland->getSpeed()) + ",";
        json += "\"brightness\":" + String(targetGarland->getBrightness());
        json += "}";
        request->send(200, "application/json", json);
    });
}