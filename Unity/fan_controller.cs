using UnityEngine;
using System.Net.Sockets;
using UnityEngine.InputSystem;

public class DeviceController : MonoBehaviour
{
    // --- NETWORK SETTINGS ---
    private UdpClient udpClient;
    private int udpPort = 8888;
    
    // IP Addresses for your two nodes
    private string ipNode1 = "192.168.50.100"; 
    private string ipNode2 = "192.168.50.101"; 

    // --- NODE 1 STATE VARIABLES (0 to 255) ---
    private float n1_fan = 0f;
    private float n1_servo1 = 127f; 
    private float n1_servo2 = 127f; 
    private int last_n1_f = -1, last_n1_s1 = -1, last_n1_s2 = -1;

    // --- NODE 2 STATE VARIABLES (0 to 255) ---
    private float n2_fan = 0f;
    private float n2_servo1 = 127f; 
    private float n2_servo2 = 127f; 
    private int last_n2_f = -1, last_n2_s1 = -1, last_n2_s2 = -1;

    // How fast the values change while holding the key
    public float adjustmentRate = 150f; 

    void Start()
    {
        udpClient = new UdpClient();
        Debug.Log("Dual Keyboard Controller Started!");
        Debug.Log("NODE 1 -> Fan: R/F | Pitch: W/S | Yaw: A/D");
        Debug.Log("NODE 2 -> Fan: U/J | Pitch: Up/Dn | Yaw: L/R");
        
        // Force an initial update to set both fans to 0 and servos to center
        ForceSendState();
    }

    void Update()
    {
        if (Keyboard.current == null) return;

        // ==========================================
        //         NODE 1 INPUTS (LEFT HAND)
        // ==========================================
        
        // Fan (R / F)
        if (Keyboard.current.rKey.isPressed) n1_fan += adjustmentRate * Time.deltaTime;
        if (Keyboard.current.fKey.isPressed) n1_fan -= adjustmentRate * Time.deltaTime;

        // Servo 1 / Pitch (W / S)
        if (Keyboard.current.wKey.isPressed) n1_servo1 += adjustmentRate * Time.deltaTime;
        if (Keyboard.current.sKey.isPressed) n1_servo1 -= adjustmentRate * Time.deltaTime;

        // Servo 2 / Yaw (D / A)
        if (Keyboard.current.dKey.isPressed) n1_servo2 += adjustmentRate * Time.deltaTime;
        if (Keyboard.current.aKey.isPressed) n1_servo2 -= adjustmentRate * Time.deltaTime;

        // Clamp Node 1 values safely between 0 and 255
        n1_fan = Mathf.Clamp(n1_fan, 0, 255);
        n1_servo1 = Mathf.Clamp(n1_servo1, 0, 255);
        n1_servo2 = Mathf.Clamp(n1_servo2, 0, 255);


        // ==========================================
        //         NODE 2 INPUTS (RIGHT HAND)
        // ==========================================
        
        // Fan (U / J)
        if (Keyboard.current.uKey.isPressed) n2_fan += adjustmentRate * Time.deltaTime;
        if (Keyboard.current.jKey.isPressed) n2_fan -= adjustmentRate * Time.deltaTime;

        // Servo 1 / Pitch (Up / Down)
        if (Keyboard.current.upArrowKey.isPressed) n2_servo1 += adjustmentRate * Time.deltaTime;
        if (Keyboard.current.downArrowKey.isPressed) n2_servo1 -= adjustmentRate * Time.deltaTime;

        // Servo 2 / Yaw (Right / Left)
        if (Keyboard.current.rightArrowKey.isPressed) n2_servo2 += adjustmentRate * Time.deltaTime;
        if (Keyboard.current.leftArrowKey.isPressed) n2_servo2 -= adjustmentRate * Time.deltaTime;

        // Clamp Node 2 values safely between 0 and 255
        n2_fan = Mathf.Clamp(n2_fan, 0, 255);
        n2_servo1 = Mathf.Clamp(n2_servo1, 0, 255);
        n2_servo2 = Mathf.Clamp(n2_servo2, 0, 255);


        // ==========================================
        //         NETWORK TRANSMISSION
        // ==========================================

        // Convert to integers for comparison
        int cur_n1_f = Mathf.RoundToInt(n1_fan);
        int cur_n1_s1 = Mathf.RoundToInt(n1_servo1);
        int cur_n1_s2 = Mathf.RoundToInt(n1_servo2);

        int cur_n2_f = Mathf.RoundToInt(n2_fan);
        int cur_n2_s1 = Mathf.RoundToInt(n2_servo1);
        int cur_n2_s2 = Mathf.RoundToInt(n2_servo2);

        // Check if Node 1 changed
        if (cur_n1_f != last_n1_f || cur_n1_s1 != last_n1_s1 || cur_n1_s2 != last_n1_s2)
        {
            last_n1_f = cur_n1_f;
            last_n1_s1 = cur_n1_s1;
            last_n1_s2 = cur_n1_s2;
            SendPWM(ipNode1, last_n1_f, last_n1_s1, last_n1_s2);
        }

        // Check if Node 2 changed
        if (cur_n2_f != last_n2_f || cur_n2_s1 != last_n2_s1 || cur_n2_s2 != last_n2_s2)
        {
            last_n2_f = cur_n2_f;
            last_n2_s1 = cur_n2_s1;
            last_n2_s2 = cur_n2_s2;
            SendPWM(ipNode2, last_n2_f, last_n2_s1, last_n2_s2);
        }
    }

    // Updated to accept the specific IP address alongside the values
    void SendPWM(string ipAddress, int fanVal, int s1Val, int s2Val)
    {
        byte[] payload = { (byte)fanVal, (byte)s1Val, (byte)s2Val };

        try 
        {
            udpClient.Send(payload, payload.Length, ipAddress, udpPort);
        } 
        catch (System.Exception e) 
        {
            Debug.LogError("Failed to send UDP to " + ipAddress + ": " + e.Message);
        }
    }

    // Bypasses the delta-check to send the baseline payload immediately on start
    void ForceSendState()
    {
        last_n1_f = Mathf.RoundToInt(n1_fan);
        last_n1_s1 = Mathf.RoundToInt(n1_servo1);
        last_n1_s2 = Mathf.RoundToInt(n1_servo2);
        SendPWM(ipNode1, last_n1_f, last_n1_s1, last_n1_s2);

        last_n2_f = Mathf.RoundToInt(n2_fan);
        last_n2_s1 = Mathf.RoundToInt(n2_servo1);
        last_n2_s2 = Mathf.RoundToInt(n2_servo2);
        SendPWM(ipNode2, last_n2_f, last_n2_s1, last_n2_s2);
    }

    void OnApplicationQuit()
    {
        if (udpClient != null) 
        {
            udpClient.Close();
        }
    }
}