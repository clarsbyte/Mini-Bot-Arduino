from openai import OpenAI
import serial
import time

ser  = serial.Serial('COM3', baudrate=115200, timeout=None)
time.sleep(2)  

client = OpenAI(
  base_url="https://openrouter.ai/api/v1",
  api_key="", 
)

def Chat(prompt):
  response = client.chat.completions.create(
    model="meta-llama/llama-4-scout:free",
    messages=[
      {
        "role": "developer",
        "content": "Give me a personal question, like asking about day for example. Format in a way that the first maximum 10 words is the question and the last two words are the choices. Example: How are you feeling? Good Bad. Then, I provide you answer, maybe Good, then you give another question, like Why are you feeling good? Friends Alone. Make it one line! Because its getting read by Arduino serial. Dont use period in the end. Please make the questions persoanlized to the first answer."
      },
        {
        "role": "user",
        "content": prompt
     }
   ]
  )
  return response.choices[0].message.content

response = ""
i = 0


while i < 3:
    if i == 0:
        a = Chat("Give me the question! Use question mark for the question but no period in choices.")
    else:
        a = Chat(f"Give me another question with the same format. Use question mark for the question but no period in choices. I answered this: {response}")
    
    print("Sent to Arduino:", a)
    ser.write((a + '\n').encode())
    time.sleep(15)
    while ser.inWaiting()>3:  
      response = ser.readline().decode('ascii').strip()
    print("Arduino response:", response)
    i += 1

ser.write('stop'.encode())