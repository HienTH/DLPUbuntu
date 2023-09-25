import requests
import time
import re
import json
from datetime import datetime

# API endpoint URL
url = "http://10.204.254.149:9090/api/loglinux/save_log_linux"

while True:

    # Open the JSON file for reading
    with open('agentInfo.json', 'r') as file:
        json_data = json.load(file)

    # Extract username and email from the JSON data
    #id_adm_member = json_data.get("id_adm_member", "")
    #id_adm_company = json_data.get("id_adm_company", "")
    #id_adm_department = json_data.get("id_adm_department", "")
    username = json_data.get("username", "")
    email = json_data.get("email", "")

    # Print the extracted information
    print("Username:", username)
    print("Email:", email)
    #print("id_adm_member:", id_adm_member)
    #print("id_adm_company:", id_adm_company)
    #print("id_adm_department:", id_adm_department)

    # Regular expression patterns for datetime and source
    datetime_pattern = r'^\w{3} \w{3} \d{2} \d{2}:\d{2}:\d{2} \d{4}'
    source_pattern = r'Source: (.+)'

    # Open the file for reading
    with open('file_transfer_log.txt', 'r') as file:
        lines = file.readlines()

    # Initialize variables to store extracted data
    extracted_data = []

    # Process the lines
    current_datetime = None
    current_source = None

    for line in lines:
        line = line.strip()

        datetime_match = re.match(datetime_pattern, line)
        source_match = re.search(source_pattern, line)

        if datetime_match:
            current_datetime = datetime_match.group()
        elif source_match:
            current_source = source_match.group(1)

            if current_datetime:
                datetime_obj = datetime.strptime(current_datetime, '%a %b %d %H:%M:%S %Y')
                formatted_datetime = datetime_obj.strftime('%Y:%m:%d %H:%M:%S')

                extracted_data.append({
                    "action": 0,
                    "file_hash": "aaaaaaaaa",
                    "file_path": "default",
                    "file_size": "1",
                    "hash": "ada89b06bf267526fe29ccd212422281",
                    "process_name": "psftp.exe",
                    'date_insert': formatted_datetime,
                    'file_name': current_source
                })

    logData = {
        "id_adm_member": 129,
        "id_adm_company": 1,
        "id_adm_department": 198,
        "log": extracted_data
    }

    print(logData)

    try:
        # Send a POST request
        response = requests.post(url, json=logData)

        # Check if the request was successful
        if response.status_code == 200:
            print("POST request successful:", response.text)
        else:
            print("POST request failed. Status code:", response.status_code)

        # Wait for 5 minutes before sending the next request
        time.sleep(1200)  # 300 seconds = 5 minutes

    except Exception as e:
        print("An error occurred:", e)