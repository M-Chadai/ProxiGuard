#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <vector>
#include <time.h>

#define RST_PIN 22
#define SS_PIN 21

MFRC522 rfid(SS_PIN, RST_PIN);
WebServer server(80);

// Access Point credentials
const char* ssid = "ESP32_Attendance";
const char* password = "123456789";

// Custom login credentials
const char* login_username = "admin";
const char* login_password = "admin123";
bool isLoggedIn = false;

struct AttendanceEntry {
  String uid;
  String name;
  String time;
};

std::vector<AttendanceEntry> attendanceLog;

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();

  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.println(WiFi.softAPIP());

  // Set time zone to Kigali (UTC+2)
  configTime(2 * 3600, 0, "pool.ntp.org"); // Offset of 2 hours for Kigali

  server.on("/", HTTP_GET, []() {
    if (!isLoggedIn) {
      server.sendHeader("Location", "/login");
      server.send(302, "text/plain", "");
    } else {
      server.send(200, "text/html", getAttendancePage());
    }
  });

  server.on("/login", HTTP_GET, []() {
    server.send(200, "text/html", getLoginPage());
  });

  server.on("/check_login", HTTP_POST, []() {
    if (!server.hasArg("username") || !server.hasArg("password")) {
      server.send(400, "text/plain", "Missing credentials");
      return;
    }

    String user = server.arg("username");
    String pass = server.arg("password");

    if (user == login_username && pass == login_password) {
      isLoggedIn = true;
      server.send(200, "text/plain", "success");
    } else {
      server.send(401, "text/plain", "Invalid credentials");
    }
  });

  server.on("/data", HTTP_GET, []() {
    if (!isLoggedIn) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }

    DynamicJsonDocument doc(2048);
    for (auto& entry : attendanceLog) {
      JsonObject obj = doc.createNestedObject();
      obj["name"] = entry.name;
      obj["time"] = entry.time;
    }

    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
  readRFID();
}

void readRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String uid = getUID(rfid.uid);
  String name;

  if (uid == "DD DC 31 03") name = "MUNYANEZA ELCHADAI";
  else if (uid == "45 89 C8 01") name = "NAMAHORO ISAACAR";
  else name = "UNKNOWN";

  String currentTime = getTimeNow();
  bool updated = false;

  for (auto& entry : attendanceLog) {
    if (entry.uid == uid) {
      entry.time = currentTime;
      updated = true;
      break;
    }
  }

  if (!updated) {
    attendanceLog.push_back({uid, name, currentTime});
  }

  Serial.printf("Scanned: %s (%s) at %s\n", name.c_str(), uid.c_str(), currentTime.c_str());
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

String getUID(MFRC522::Uid uid) {
  char buffer[20];
  sprintf(buffer, "%02X %02X %02X %02X", uid.uidByte[0], uid.uidByte[1], uid.uidByte[2], uid.uidByte[3]);
  return String(buffer);
}

String getTimeNow() {
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);
  char timeStr[25];
  sprintf(timeStr, "%04d-%02d-%02d %02d:%02d:%02d", 
          t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
          t->tm_hour, t->tm_min, t->tm_sec);
  return String(timeStr);
}

String getLoginPage() {
  return R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Login - Smart Attendance</title>
      <style>
        body {
          font-family: Arial, sans-serif;
          display: flex;
          justify-content: center;
          align-items: center;
          height: 100vh;
          margin: 0;
          background-image: url('https://www.w3schools.com/w3images/lights.jpg');
          background-size: cover;
          background-position: center;
        }
        form {
          background: rgba(255, 255, 255, 0.9);
          padding: 20px;
          border-radius: 10px;
          box-shadow: 0 2px 10px rgba(0,0,0,0.2);
        }
        input, button {
          width: 100%;
          padding: 10px;
          margin: 10px 0;
          border-radius: 5px;
        }
        button {
          background: #4CAF50;
          color: white;
          border: none;
        }
        .error {
          color: red;
          text-align: center;
        }
      </style>
      <script>
        function login(event) {
          event.preventDefault();
          const username = document.getElementById("username").value;
          const password = document.getElementById("password").value;

          fetch("/check_login", {
            method: "POST",
            headers: { "Content-Type": "application/x-www-form-urlencoded" },
            body: `username=${username}&password=${password}`
          }).then(response => {
            if (response.status === 200) {
              window.location.href = "/"; 
            } else {
              document.getElementById("error").innerText = "Invalid login!";
            }
          });
        }
      </script>
    </head>
    <body>
      <form onsubmit="login(event)">
        <h2>Login</h2>
        <input id="username" type="text" placeholder="Username" required>
        <input id="password" type="password" placeholder="Password" required>
        <button type="submit">Login</button>
        <p class="error" id="error"></p>
      </form>
    </body>
    </html>
  )rawliteral";
}

String getAttendancePage() {
  return R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Smart Attendance</title>
      <style>
        body {
          font-family: 'Arial', sans-serif;
          background-image: url('https://www.w3schools.com/w3images/lights.jpg');
          background-size: cover;
          background-position: center;
          padding: 20px;
          margin: 0;
        }
        h1 {
          text-align: center;
          color: #fff;
          text-shadow: 1px 1px 3px #000;
        }
        table {
          margin: 0 auto;
          border-collapse: collapse;
          width: 90%;
          background-color: rgba(255, 255, 255, 0.95);
          box-shadow: 0 0 10px rgba(0,0,0,0.3);
        }
        th, td {
          border: 1px solid #ccc;
          padding: 12px;
          text-align: center;
        }
        th {
          background-color: #4CAF50;
          color: white;
        }
        tr:nth-child(even) {
          background-color: #f2f2f2;
        }
      </style>
      <script>
        setInterval(() => {
          fetch("/data")
            .then(res => res.json())
            .then(data => {
              const tbody = document.getElementById("attendance-body");
              tbody.innerHTML = "";
              data.forEach(entry => {
                const row = `<tr><td>${entry.name}</td><td>${entry.time}</td></tr>`;
                tbody.innerHTML += row;
              });
            });
        }, 2000);
      </script>
    </head>
    <body>
      <h1>Smart Attendance</h1>
      <table>
        <thead>
          <tr>
            <th>Name</th>
            <th>Time</th>
          </tr>
        </thead>
        <tbody id="attendance-body"></tbody>
      </table>
    </body>
    </html>
  )rawliteral";
}
