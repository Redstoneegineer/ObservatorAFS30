import time
import serial
import random
import string

def calculate_checksum(data):
    """Bereken de XOR-checksum voor een gegeven string."""
    checksum = 0
    for char in data:
        checksum ^= ord(char)
    return checksum

def generate_nmea_sentence(error_type):
    """Genereer een NMEA-zin met een specifieke fout."""
    sentence_body = "GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W"
    if error_type == "checksum":
        # Onjuiste checksum
        checksum = 0x00
    elif error_type == "format":
        # Ontbrekende asterisk voor checksum
        return f"${sentence_body}{calculate_checksum(sentence_body):02X}\r\n"
    elif error_type == "invalid_char":
        # Bevat niet-printbare ASCII-tekens
        sentence_body = "GPRMC,123519,A,4807.038,N,01131.000,E,\x01\x02,084.4,230394,003.1,W"
        checksum = calculate_checksum(sentence_body)
    elif error_type == "length":
        # Overschrijdt maximale lengte (bijv. 100 tekens)
        long_field = "X" * 90
        sentence_body = f"GPRMC,{long_field}"
        checksum = calculate_checksum(sentence_body)
    else:
        # Geen fout, geldig NMEA-zin
        checksum = calculate_checksum(sentence_body)
    return f"${sentence_body}*{checksum:02X}\r\n"

def generate_tag_block(error_type, counter):
    """Genereer een TAG-blok met een specifieke fout."""
    timestamp = int(time.time())
    tag_content = f"c:{timestamp},n:{counter}"

    if error_type == "framing":
        # Ontbrekende begin-backslash
        tag_block = f"{tag_content}*{calculate_checksum(tag_content):02X}\\"
    elif error_type == "length":
        # Overschrijdt maximale lengte (bijv. 100 tekens)
        long_content = "c:" + "X" * 95
        tag_block = f"\\{long_content}*{calculate_checksum(long_content):02X}\\"
    elif error_type == "format":
        # Ontbrekende asterisk voor checksum
        tag_block = f"\\{tag_content}{calculate_checksum(tag_content):02X}\\"
    elif error_type == "invalid_char":
        # Bevat niet-printbare ASCII-tekens
        invalid_content = f"c:{chr(0x01)}{chr(0x02)}"
        tag_block = f"\\{invalid_content}*{calculate_checksum(invalid_content):02X}\\"
    elif error_type == "checksum":
        # Onjuiste checksum
        tag_block = f"\\{tag_content}*00\\"
    else:
        # Geen fout, geldig TAG-blok
        tag_block = f"\\{tag_content}*{calculate_checksum(tag_content):02X}\\"

    return tag_block

def generate_test_messages(batch_size=50):
    """Genereer NMEA-zinnen met verschillende TAG-blokfouten."""
    tag_error_types = ["framing", "length", "format", "invalid_char", "checksum", "none"]
    nmea_error_types = ["length", "checksum", "invalid_char", "format", "none"]
    messages = []

    for i in range(batch_size):
        if i % 2 == 0:
            # Even index: fout in TAG-blok, geldige NMEA-zin
            tag_error = tag_error_types[i % len(tag_error_types)]
            nmea_error = "none"
        else:
            # Oneven index: geldige TAG-blok, fout in NMEA-zin
            tag_error = "none"
            nmea_error = nmea_error_types[i % len(nmea_error_types)]
        tag_block = generate_tag_block(tag_error, i)
        nmea_sentence = generate_nmea_sentence(nmea_error)
        full_message = f"{tag_block}{nmea_sentence}"
        messages.append((i, tag_error, nmea_error, full_message))

    return messages

if __name__ == "__main__":
    # Pas de seriële poort aan indien nodig
    try:
        with serial.Serial(port="COM8", baudrate=115200, timeout=1) as ser:
            test_messages = generate_test_messages()
            tag_error_counts = {}
            nmea_error_counts = {}
            for counter, tag_error, nmea_error, message in test_messages:
                print(f"Bericht {counter + 1}: TAG-fout - {tag_error}, NMEA-fout - {nmea_error}")
                print(message)
                print(f"Lengte: {len(message.strip())} tekens\n")
                ser.write(message.encode('ascii'))
                time.sleep(0.5)
                # Statistieken bijwerken
                tag_error_counts[tag_error] = tag_error_counts.get(tag_error, 0) + 1
                nmea_error_counts[nmea_error] = nmea_error_counts.get(nmea_error, 0) + 1
            # Statistieken afdrukken
            print("\nStatistieken:")
            print("TAG-fouten:")
            for error_type, count in tag_error_counts.items():
                print(f"  {error_type}: {count}")
            print("NMEA-fouten:")
            for error_type, count in nmea_error_counts.items():
                print(f"  {error_type}: {count}")
    except serial.SerialException as e:
        print(f"Fout bij het openen van de seriële poort: {e}")
