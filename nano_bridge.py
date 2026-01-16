DATASET_ID = "cpc357_project"
TABLE_ID = "detection_table"

bq_client = bigquery.Client()
table_ref = f"{PROJECT_ID}.{DATASET_ID}.{TABLE_ID}"

# This triggers when the script successfully connects to Mosquitto
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("✅  Connected to Mosquitto Broker!")
        client.subscribe("urban/noise") # Subscribe HERE, not outside
    else:
        print(f"❌  Failed to connect, return code {rc}")

def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode("utf-8"))
        print(f"📩  Received Message: {payload}")

        # Stream to BigQuery
        errors = bq_client.insert_rows_json(table_ref, [payload])

        if not errors:
            print(f"🚀  Successfully stored labels: {payload['labels']}")
        else:
            # THIS IS CRUCIAL: BigQuery tells you exactly why it failed here
            print(f"🔥  BigQuery Schema Error: {errors}")

    except Exception as e:
        print(f"⚠️ Script Error: {e}")

mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect # Register the connection callback
mqtt_client.on_message = on_message

mqtt_client.connect("localhost", 1883)
print("Bridge is starting...")
mqtt_client.loop_forever() # Keep it running

