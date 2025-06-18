import time
import serial

def calculate_checksum(data):
    """Bereken de XOR-checksum voor een gegeven string."""
    checksum = 0
    for char in data:
        checksum ^= ord(char)
    return checksum

def generate_nmea_sentence():
    """Genereer een geldige NMEA-zin zonder fouten."""
    sentence_body = "GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W"
    checksum = calculate_checksum(sentence_body)
    return f"${sentence_body}*{checksum:02X}\r\n"

def generate_tag_block(counter):
    """Genereer een geldig TAG-blok zonder fouten."""
    timestamp = int(time.time())
    tag_content = f"c:{timestamp},n:{counter}"
    tag_block = f"\\{tag_content}*{calculate_checksum(tag_content):02X}\\"
    return tag_block

def main():
    """Verstuur 1 geldig NMEA-bericht en 1 geldig TAG-blok met een pauze van 2 seconden tussen elke verzending."""
    try:
        # Pas de seriële poort aan indien nodig
        with serial.Serial(port="COM8", baudrate=115200, timeout=1) as ser:
            counter = 1
            while True:  # Blijf berichten sturen
                # Genereer de berichten
                tag_block = generate_tag_block(counter)
                nmea_sentence = generate_nmea_sentence()
                full_message = f"{tag_block}{nmea_sentence}"

                # Print het bericht naar de console voor debuggen
                print(f"Bericht {counter}:")
                print(f"TAG-blok: {tag_block}")
                print(f"NMEA-zin: {nmea_sentence}")
                print(f"Volledig bericht: {full_message}")

                # Verstuur het bericht via de seriële poort
                ser.write(full_message.encode('ascii'))  # Verstuur het bericht
                print("Bericht verstuurd!")

                counter += 1  # Verhoog de counter voor het volgende bericht

                # Wacht 2 seconden voordat het volgende bericht wordt verstuurd
                #time.sleep(2)

    except serial.SerialException as e:
        print(f"Fout bij het openen van de seriële poort: {e}")

if __name__ == "__main__":
    main()
