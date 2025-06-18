import time
import serial

def calculate_checksum(data):
    """Bereken de XOR-checksum voor een gegeven string."""
    checksum = 0
    for char in data:
        checksum ^= ord(char)
    return checksum

def generate_nmea_sentence():
    """Genereer een GELDIGE NMEA-zin (GPRMC-voorbeeld)."""
    sentence_body = "GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W"
    checksum = calculate_checksum(sentence_body)
    return f"${sentence_body}*{checksum:02X}\r\n"

def generate_tag_block(counter):
    """Genereer een GELDIG TAG-blok."""
    timestamp = int(time.time())
    tag_content = f"c:{timestamp},n:{counter}"
    checksum = calculate_checksum(tag_content)
    return f"\\{tag_content}*{checksum:02X}\\"

def generate_valid_messages(batch_size=50):
    """Genereer alleen geldige NMEA-zinnen met geldige TAG-blokken."""
    messages = []
    for i in range(batch_size):
        tag_block = generate_tag_block(i)
        nmea_sentence = generate_nmea_sentence()
        full_message = f"{tag_block}{nmea_sentence}"
        messages.append((i, "none", "none", full_message))  # Geen fouten
    return messages

if __name__ == "__main__":
    # Pas de seriële poort aan indien nodig
    try:
        with serial.Serial(port="COM8", baudrate=115200, timeout=1) as ser:
            test_messages = generate_valid_messages()
            for counter, tag_error, nmea_error, message in test_messages:
                print(f"Bericht {counter + 1}: (Geen fouten)")
                print(message)
                print(f"Lengte: {len(message.strip())} tekens\n")
                ser.write(message.encode('ascii'))
                time.sleep(0.5)
            
            print("\nStatistieken:")
            print("Alle berichten waren geldig (geen TAG- of NMEA-fouten).")
    except serial.SerialException as e:
        print(f"Fout bij het openen van de seriële poort: {e}")