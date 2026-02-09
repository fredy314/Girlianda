#include "PagesHandlers.h"
#include <Arduino.h>

PagesHandlers::PagesHandlers(Garland& garland)
    : _garland(garland)
{}

void PagesHandlers::initPagesHandlers(AsyncWebServer& webServer) {
    setupHomePageHandler(webServer);
    setupApiStatusHandler(webServer);
    setupApiSetHandler(webServer);
}

const char HTML_HOME[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="uk">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Керування Гірляндою</title>
  <style>
    body { font-family: sans-serif; text-align: center; background: #121212; color: #ffffff; padding: 20px; }
    h1 { color: #f39c12; }
    .card { background: #1e1e1e; padding: 20px; border-radius: 10px; max-width: 400px; margin: 0 auto; box-shadow: 0 4px 6px rgba(0,0,0,0.3); }
    .btn { display: block; width: 100%; padding: 15px; margin: 10px 0; background: #34495e; color: white; border: none; border-radius: 5px; font-size: 18px; cursor: pointer; transition: 0.3s; }
    .btn.active { background: #e74c3c; font-weight: bold; }
    .btn:hover { background: #455a64; }
    .slider-container { margin: 20px 0; }
    input[type=range] { width: 100%; }
    label { display: block; margin-bottom: 5px; font-size: 14px; color: #bbb; }
    #status { margin-top: 15px; font-size: 12px; color: #7f8c8d; }
    .ota-link { background: #8e44ad; margin-top: 15px; text-decoration: none; }
    .ota-link:hover { background: #9b59b6; }
  </style>
</head>
<body>
  <div class="card">
    <h1>🎄 Гірлянда 🎄</h1>
    
    <label>Режим</label>
    <button class="btn" onclick="setMode(0)" id="btn-0">Вимкнено</button>
    <button class="btn" onclick="setMode(1)" id="btn-1">Постійне світло</button>
    <button class="btn" onclick="setMode(2)" id="btn-2">Протифаза (Альт)</button>
    <button class="btn" onclick="setMode(3)" id="btn-3">Синхронне дихання</button>
    <button class="btn" onclick="setMode(4)" id="btn-4">Хаос</button>
    <button class="btn" onclick="setMode(5)" id="btn-5">Мерехтіння</button>

    <div class="slider-container">
      <label for="speed">Швидкість анімації: <span id="speed-val">50</span>%</label>
      <input type="range" id="speed" min="1" max="100" value="50" oninput="updateSpeed(this.value)" onchange="setSpeed(this.value)">
    </div>

    <div class="slider-container">
      <label for="bright">Яскравість (для постійного): <span id="bright-val">255</span></label>
      <input type="range" id="bright" min="0" max="255" value="255" oninput="updateBright(this.value)" onchange="setBright(this.value)">
    </div>
    
    <a href="/update" class="btn ota-link" target="_blank">🔄 Оновити</a>
    
    <div id="status">Підключення...</div>
  </div>

<script>
function updateSpeed(val) { document.getElementById('speed-val').innerText = val; }
function updateBright(val) { document.getElementById('bright-val').innerText = val; }

function setMode(m) {
  fetch('/api/set?mode=' + m)
    .then(r => updateState());
}

function setSpeed(s) {
  fetch('/api/set?speed=' + s); // Fire and forget logic for smoother slider, or update state after
}

function setBright(b) {
  fetch('/api/set?brightness=' + b);
}

function updateState() {
  fetch('/api/status')
    .then(response => response.json())
    .then(data => {
      // Update buttons
      for(let i=0; i<=5; i++) {
        let btn = document.getElementById('btn-'+i);
        if(data.mode == i) btn.classList.add('active');
        else btn.classList.remove('active');
      }
      
      // Update sliders
      document.getElementById('speed').value = data.speed;
      document.getElementById('speed-val').innerText = data.speed;
      
      document.getElementById('bright').value = data.brightness;
      document.getElementById('bright-val').innerText = data.brightness;

      document.getElementById('status').innerText = 'Останнє оновлення: ' + new Date().toLocaleTimeString();
    })
    .catch(err => {
      document.getElementById('status').innerText = 'Помилка з\'єднання';
    });
}

// Poll status every 2 seconds
setInterval(updateState, 2000);
// Initial load
updateState();
</script>
</body>
</html>
)rawliteral";

void PagesHandlers::setupHomePageHandler(AsyncWebServer& webServer) {
    webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", HTML_HOME);
    });
}

void PagesHandlers::setupApiStatusHandler(AsyncWebServer& webServer) {
    webServer.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request){
        String json = "{";
        json += "\"mode\":" + String(_garland.getMode()) + ",";
        json += "\"speed\":" + String(_garland.getSpeed()) + ",";
        json += "\"brightness\":" + String(_garland.getBrightness());
        json += "}";
        request->send(200, "application/json", json);
    });
}

void PagesHandlers::setupApiSetHandler(AsyncWebServer& webServer) {
    webServer.on("/api/set", HTTP_GET, [this](AsyncWebServerRequest *request){
        if (request->hasParam("mode")) {
            _garland.setMode(request->getParam("mode")->value().toInt());
        }
        if (request->hasParam("speed")) {
            _garland.setSpeed(request->getParam("speed")->value().toInt());
        }
        if (request->hasParam("brightness")) {
            _garland.setBrightness(request->getParam("brightness")->value().toInt());
        }
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });
}