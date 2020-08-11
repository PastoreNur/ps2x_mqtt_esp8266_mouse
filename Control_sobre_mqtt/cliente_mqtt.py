import paho.mqtt.client as mqtt
import json
from pynput.mouse import Button, Controller

mouse = Controller()

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("Prueba")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    
    if msg.payload == b'Izquierdo':
        mouse.click(Button.left, 1)
    if msg.payload == b'Derecho':
        mouse.click(Button.right, 1)  
    payloadjson = json.loads(msg.payload)
    if int(payloadjson["Y"]) != 128 or int(payloadjson["X"]) != 128:
        mouse.move(int(payloadjson["X"])-128, int(payloadjson["Y"])-128)
    
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("192.168.0.18", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()