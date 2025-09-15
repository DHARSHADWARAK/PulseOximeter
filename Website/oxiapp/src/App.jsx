import React, { useState, useEffect } from "react";

const ESP32_IP_ADDRESS = "http://192.168.162.215"; // Replace with your ESP32 IP

// Returns status text and color class for heart rate
function getHeartRateStatus(hr) {
  if (hr < 50) return { text: "Bradycardia (danger if not an athlete)", color: "danger" };
  if (hr <= 100) return { text: "Normal Resting", color: "normal" };
  if (hr <= 120) return { text: "Elevated (monitor)", color: "warning" };
  if (hr <= 180) return { text: "Tachycardia (could be abnormal if resting)", color: "alert" };
  return { text: "Danger (severe tachycardia)", color: "danger" };
}

// Returns status text and color class for SpO2
function getSpO2Status(spo2) {
  if (spo2 >= 95) return { text: "Normal", color: "normal" };
  if (spo2 >= 90) return { text: "Mildly Low (monitor)", color: "warning" };
  if (spo2 >= 85) return { text: "Danger (hypoxemia)", color: "alert" };
  return { text: "Critical (serious condition)", color: "danger" };
}

function App() {
  const [data, setData] = useState({ heartRate: 0, spo2: 0, countdown: 10 });
  const [error, setError] = useState(null);

  const fetchSensorData = async () => {
    try {
      const response = await fetch(`${ESP32_IP_ADDRESS}/data`);
      if (!response.ok) {
        throw new Error("Failed to fetch data from ESP32");
      }
      const json = await response.json();
      setData({
        heartRate: parseFloat(json.heartRate) || 0,
        spo2: parseFloat(json.spo2) || 0,
        countdown: json.countdown || 10,
      });
      setError(null);
    } catch (err) {
      setError("Could not get ESP32 data.");
    }
  };

  useEffect(() => {
    fetchSensorData();
    const interval = setInterval(fetchSensorData, 2000);
    return () => clearInterval(interval);
  }, []);

  const hrStatus = getHeartRateStatus(data.heartRate);
  const spo2Status = getSpO2Status(data.spo2);

  return (
    <>
    <div className="flex-box">
      
      <div className="dashboard">
        <h2>MAX30100 ESP32 Monitor</h2>
        {error && <p style={{ color: "red", textAlign: "center" }}>{error}</p>}
        <div className="data-row">
          <span className="data-label">Heart Rate:</span>
          <span className="data-value">{data.heartRate} bpm</span>
        </div>
        <div className="data-row">
          <span className="data-label">SpO<sub>2</sub>:</span>
          <span className="data-value">{data.spo2} %</span>
        </div>
        <div className="data-row">
          <span className="data-label">Countdown:</span>
          <span className="data-value">{data.countdown}</span>
        </div>
      </div>

      <div className="status-container">
        <div className={`status-box ${hrStatus.color}`}>
          <h3>Heart Rate Status</h3>
          <p>{hrStatus.text}</p>
        </div>
        <div className={`status-box ${spo2Status.color}`}>
          <h3>SpO<sub>2</sub> Status</h3>
          <p>{spo2Status.text}</p>
        </div>
      </div>
    </div>
    </>
  );
}

export default App;
