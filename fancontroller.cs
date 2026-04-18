using UnityEngine;
using System.Net.Sockets;
using UnityEngine.InputSystem; 

public class DeviceController : MonoBehaviour
{
    // --- NETWORK SETTINGS ---
    private UdpClient udpClient;
    private int udpPort = 8888;
    private string espIpAddress = "192.168.50.100"; 

    // --- STATE VARIABLES (0 to 255) ---
    private float fan1Value = 0f;
    private float servo1Value = 127f; // Start servos in the middle
    private float servo2Value = 127f; 

    // How fast the values change while holding the key
    public float adjustmentRate = 150f; 

    // Keep track of the last sent values to avoid network spam
    private int lastF1 = -1, lastS1 = -1, lastS2 = -1;

    void Start()
    {
        udpClient = new UdpClient();
        Debug.Log("Controller Started! Q/A: Fan | W/S: Servo 1 | E/D: Servo 2");
        SendPWM(); // Send initial center/off state
    }

    void Update()
    {
        if (Keyboard.current == null) return;

        // --- FAN 1 (Q / A) ---
        if (Keyboard.current.qKey.isPressed) fan1Value += adjustmentRate * Time.deltaTime;
        if (Keyboard.current.aKey.isPressed) fan1Value -= adjustmentRate * Time.deltaTime;

        // --- SERVO 1 (W / S) ---
        if (Keyboard.current.wKey.isPressed) servo1Value += adjustmentRate * Time.deltaTime;
        if (Keyboard.current.sKey.isPressed) servo1Value -= adjustmentRate * Time.deltaTime;

        // --- SERVO 2 (E / D) ---
        if (Keyboard.current.eKey.isPressed) servo2Value += adjustmentRate * Time.deltaTime;
        if (Keyboard.current.dKey.isPressed) servo2Value -= adjustmentRate * Time.deltaTime;

        // Clamp values safely between 0 and 255
        fan1Value = Mathf.Clamp(fan1Value, 0, 255);
        servo1Value = Mathf.Clamp(servo1Value, 0, 255);
        servo2Value = Mathf.Clamp(servo2Value, 0, 255);

        // Convert back to integers
        int currentF1 = Mathf.RoundToInt(fan1Value);
        int currentS1 = Mathf.RoundToInt(servo1Value);
        int currentS2 = Mathf.RoundToInt(servo2Value);

        // Only send a UDP packet if the integer value has actually shifted
        if (currentF1 != lastF1 || currentS1 != lastS1 || currentS2 != lastS2)
        {
            lastF1 = currentF1;
            lastS1 = currentS1;
            lastS2 = currentS2;
            SendPWM();
        }
    }

    void SendPWM()
    {
        byte[] payload = { (byte)lastF1, (byte)lastS1, (byte)lastS2 };

        try 
        {
            udpClient.Send(payload, payload.Length, espIpAddress, udpPort);
        } 
        catch (System.Exception e) 
        {
            Debug.LogError("Failed to send UDP: " + e.Message);
        }
    }

    void OnApplicationQuit()
    {
        if (udpClient != null) 
        {
            udpClient.Close();
        }
    }
}