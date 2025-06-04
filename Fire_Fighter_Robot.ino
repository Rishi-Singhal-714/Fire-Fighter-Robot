#include <WiFi.h>
#include <ESP32Servo.h>

// Motor control pins
const int in1 = 26;
const int in2 = 25;
const int in3 = 21;
const int in4 = 22;

// Relay pin (you said you’re already using GPIO 23)
const int relayPin = 23;

// Ultrasonic sensor pins
const int trigPin = 14;
const int echoPin = 12;

// Flame sensor pins (analog)
const int flameLeft   = 35;
const int flameRight  = 32;
const int flameCenter = 34;

// Servo for pipe
const int servoPin = 13;
Servo waterServo;

// Wi-Fi credentials
const char* ssid     = "OnePlus 12R";
const char* password = "12345678";

WiFiServer server(80);
bool autoMode = false;

// Read ultrasonic distance (cm)
long readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long d = pulseIn(echoPin, HIGH, 30000);
  if (!d) return 999;
  return d * 0.034 / 2;
}

// Motor helpers
void forward() {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
}

void backward() {
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
}

void turnRight() {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);  digitalWrite(in4, HIGH);
}

void turnLeft() {
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
}

void stopMotors() {
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);
  stopMotors();

  pinMode(relayPin, OUTPUT);        // relay is now purely manual
  digitalWrite(relayPin, HIGH);      // ensure it starts OFF
  
  // Ultrasonic
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Flame sensors (still used for movement logic)
  pinMode(flameLeft, INPUT);
  pinMode(flameRight, INPUT);
  pinMode(flameCenter, INPUT);

  // Servo
  waterServo.attach(servoPin);
  waterServo.write(90);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nConnected – IP = " + WiFi.localIP().toString());
  server.begin();
}

void loop() {
  // 1) Obstacle check
  long dist = readDistance();
  Serial.printf("Distance: %ld cm\n", dist);
  if (dist < 30) {
    Serial.println("  Obstacle! Stopping.");
    stopMotors();
    // relay untouched
  }

  // 2) Flame-chasing movement (relay logic removed entirely)
  int vLeft   = analogRead(flameLeft);
  int vCenter = analogRead(flameCenter);
  int vRight  = analogRead(flameRight);
  Serial.printf("Flames L/C/R = %d / %d / %d\n", vLeft, vCenter, vRight);

  if (!(vLeft == 0 || vCenter == 0 || vRight == 0)) {
    const int TH = 1000;
    if (vCenter < TH) {
      autoMode = true;
      dist = readDistance();
      if (dist < 30) {
        Serial.println("  Obstacle during auto-mode. Stopping.");
        stopMotors();
        autoMode = false;
      } else {
        waterServo.write(90);
        Serial.println(" -> Center flame: moving forward");
        forward();
      }
      delay(200);
    }
    else if (vLeft < TH || vRight < TH) {
      autoMode = true;
      dist = readDistance();
      if (dist < 30) {
        Serial.println("  Obstacle during auto-mode. Stopping.");
        stopMotors();
        autoMode = false;
      } else {
        int diff = abs(vLeft - vRight);
        if (diff < 50) {
          waterServo.write(90);
          Serial.println(" -> Flame equal: moving forward");
          forward();
        }
        else if (vLeft < vRight) {
          waterServo.write(60);
          Serial.println(" -> Flame left: turning left");
          turnLeft();
        }
        else {
          waterServo.write(120);
          Serial.println(" -> Flame right: turning right");
          turnRight();
        }
      }
      delay(200);
    }
    else if (autoMode) {
      Serial.println("  No flame: stopping auto");
      stopMotors();
      autoMode = false;
    }
  }

  // 3) Web server
  WiFiClient client = server.available();
  if (!client) return;

  String req = client.readStringUntil('\r');
  client.read(); // consume '\n'
  Serial.println("Request: " + req);

  // Motor commands
       if (req.indexOf("GET /forward")  >= 0) forward();
  else if (req.indexOf("GET /backward") >= 0) backward();
  else if (req.indexOf("GET /left")     >= 0) turnLeft();
  else if (req.indexOf("GET /right")    >= 0) turnRight();
  else if (req.indexOf("GET /stop")     >= 0) stopMotors();

  // Manual relay commands
  else if (req.indexOf("GET /relayOn")  >= 0) {
    digitalWrite(relayPin, LOW);
    Serial.println("Web → Relay ON");
  }
  else if (req.indexOf("GET /relayOff") >= 0) {
    digitalWrite(relayPin, HIGH);
    Serial.println("Web → Relay OFF");
  }

  // 4) Respond with HTML
  client.println("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
  client.println(R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Fire Fighter Robot</title>
  <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@400;500;600&display=swap" rel="stylesheet">
  <style>
    :root {
      --primary: #28a745;
      --primary-hover: #218838;
      --danger: #dc3545;
      --danger-hover: #c82333;
      --light: #ffffff;
      --dark: #1f2937;
      --bg: #f8fafc;
      --card-bg: #ffffff;
      --radius: 14px;
      --transition: all 0.3s ease;
      --shadow: 0 4px 20px rgba(0, 0, 0, 0.1);
      --hover-shadow: 0 6px 24px rgba(0, 0, 0, 0.15);
    }

    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }

    body {
      font-family: 'Poppins', 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: var(--bg);
      color: var(--dark);
      display: flex;
      flex-direction: column;
      align-items: center;
      min-height: 100vh;
      padding: 30px 20px;
    }

    .header {
      text-align: center;
      margin-bottom: 30px;
    }

    .header h1 {
      font-size: 2.5rem;
      font-weight: 600;
      margin-bottom: 10px;
    }

    .header .logo {
      max-width: 180px;
      height: auto;
      margin-top: 10px;
    }

    .card {
      background: var(--card-bg);
      padding: 30px 20px;
      border-radius: var(--radius);
      box-shadow: var(--shadow);
      width: 100%;
      max-width: 420px;
    }

    .controller {
      display: grid;
      grid-template-areas:
        ". up ."
        "left . right"
        ". down .";
      gap: 15px;
      justify-items: center;
      align-items: center;
      margin: 30px 0;
    }

    .controller button {
      width: 70px;
      height: 70px;
      font-size: 1.8rem;
      border: none;
      border-radius: var(--radius);
      background: var(--card-bg);
      box-shadow: var(--shadow);
      cursor: pointer;
      transition: var(--transition);
      display: flex;
      align-items: center;
      justify-content: center;
    }

    .controller button:hover {
      transform: translateY(-4px);
      box-shadow: var(--hover-shadow);
    }

    .up    { grid-area: up; }
    .down  { grid-area: down; }
    .left  { grid-area: left; }
    .right { grid-area: right; }

    .manual-controls {
      display: flex;
      justify-content: center;
      margin-top: 30px;
    }

    .manual-controls button {
      padding: 14px 28px;
      font-size: 1rem;
      border-radius: var(--radius);
      border: none;
      cursor: pointer;
      transition: var(--transition);
      box-shadow: var(--shadow);
      font-weight: 500;
      background: var(--danger);
      color: var(--light);
    }

    .manual-controls button.active {
      background: var(--primary);
    }

    .manual-controls button:hover {
      transform: translateY(-3px);
      box-shadow: var(--hover-shadow);
    }
    @media (max-width: 768px) {
  .card {
    padding: 20px 15px;
    max-width: 90%;
  }

  .controller button {
    width: 60px;
    height: 60px;
    font-size: 1.5rem;
  }

  .manual-controls button {
    padding: 12px 24px;
    font-size: 0.95rem;
  }

  .header h1 {
    font-size: 2rem;
  }

  .header .logo {
    max-width: 140px;
  }
}

@media (max-width: 480px) {
  .controller {
    gap: 12px;
  }

  .controller button {
    width: 50px;
    height: 50px;
    font-size: 1.3rem;
  }

  .manual-controls button {
    padding: 10px 20px;
    font-size: 0.9rem;
  }

  .header h1 {
    font-size: 1.6rem;
  }

  .header .logo {
    max-width: 120px;
  }
}
  </style>
</head>

<body>
  <div class="header">
    <h1>Fire Fighter Robot</h1>
    <img src="https://vips.edu/assets/logo-e78ff0ab.webp" alt="VIPS Logo" class="logo">
  </div>

  <div class="card">
    <div class="controller">
      <button class="up" id="btnUp">↑</button>
      <button class="left" id="btnLeft">←</button>
      <button class="right" id="btnRight">→</button>
      <button class="down" id="btnDown">↓</button>
    </div>

    <div class="manual-controls">
      <button id="relayToggle">Relay OFF</button>
    </div>
  </div>

  <script>
    const send = path => fetch(path);

    [['btnUp', '/forward'], ['btnDown', '/backward'],
     ['btnLeft', '/left'], ['btnRight', '/right']].forEach(([id, cmd]) => {
      const btn = document.getElementById(id);
      btn.addEventListener('mousedown', () => send(cmd));
      btn.addEventListener('touchstart', () => send(cmd));
      btn.addEventListener('mouseup', () => send('/stop'));
      btn.addEventListener('touchend', () => send('/stop'));
    });

    const relayBtn = document.getElementById('relayToggle');
    let relayState = false; // false = OFF, true = ON

    relayBtn.addEventListener('click', () => {
      relayState = !relayState;
      if (relayState) {
        send('/relayOn');
        relayBtn.textContent = 'Relay ON';
        relayBtn.classList.add('active');
      } else {
        send('/relayOff');
        relayBtn.textContent = 'Relay OFF';
        relayBtn.classList.remove('active');
      }
    });
  </script>
</body>
</html>
  )rawliteral");
  client.stop();
}
