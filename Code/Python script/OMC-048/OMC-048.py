import time
import omc048

# Initialize Serial interface for RS232
s1 = omc048.serial(1)
s1.init(19200, s1.RS422)

# Functie om de XOR-checksum te berekenen
def calculate_checksum(data):
    checksum = 0
    for char in data:
        checksum ^= ord(char)
    return checksum

# Genereer de NMEA-zin voor de windsensor met een specifieke fout
def generate_nmea_sentence(error_type):
    sentence_body = "WIMWV,1.2,R,10.5,M,A"  # Windsensor output
    if error_type == "checksum":
        # Onjuiste checksum
        checksum = 0x00
    elif error_type == "format":
        # Ontbrekende asterisk voor checksum
        return "$" + "{}*{:02X}\r\n".format(sentence_body, calculate_checksum(sentence_body))
    elif error_type == "invalid_char":
        # Bevat niet-printbare ASCII-tekens
        sentence_body = "WIMWV,1.2,R,10.5,M,\x01\x02,A"
        checksum = calculate_checksum(sentence_body)
    elif error_type == "length":
        # Overschrijdt maximale lengte (bijv. 100 tekens)
        long_field = "X" * 90
        sentence_body = "WIMWV,{}".format(long_field)
        checksum = calculate_checksum(sentence_body)
    else:
        # Geen fout, geldig NMEA-zin
        checksum = calculate_checksum(sentence_body)
    return "$" + "{}*{:02X}\r\n".format(sentence_body, checksum)

# Genereer het TAG-blok met een specifieke fout
def generate_tag_block(error_type, counter):
    timestamp = int(time.time())
    tag_content = "c:{},n:{}".format(timestamp, counter)

    if error_type == "framing":
        # Ontbrekende begin-backslash
        tag_block = "{}*{:02X}\\".format(tag_content, calculate_checksum(tag_content))
    elif error_type == "length":
        # Overschrijdt maximale lengte (bijv. 100 tekens)
        long_content = "c:" + "X" * 95
        tag_block = "\\{}*{:02X}\\".format(long_content, calculate_checksum(long_content))
    elif error_type == "format":
        # Ontbrekende asterisk voor checksum
        tag_block = "\\{}{:02X}\\".format(tag_content, calculate_checksum(tag_content))
    elif error_type == "invalid_char":
        # Bevat niet-printbare ASCII-tekens
        invalid_content = "c:{}{}".format(chr(0x01), chr(0x02))
        tag_block = "\\{}*{:02X}\\".format(invalid_content, calculate_checksum(invalid_content))
    elif error_type == "checksum":
        # Onjuiste checksum
        tag_block = "\\{}*00\\".format(tag_content)
    else:
        # Geen fout, geldig TAG-blok
        tag_block = "\\{}*{:02X}\\".format(tag_content, calculate_checksum(tag_content))

    return tag_block

# Genereer een lijst van testberichten met verschillende NMEA en TAG-blokfouten
def generate_test_messages(batch_size=50):
    tag_error_types = ["framing", "length", "format", "invalid_char", "checksum", "none"]
    nmea_error_types = ["length", "checksum", "invalid_char", "format", "none"]
    messages = []

    for i in range(batch_size):
        if i % 5 == 0:
            # Elke 5e bericht is geldig (NMEA + TAG-blok zonder fouten)
            tag_error = "none"
            nmea_error = "none"
        else:
            # Andere berichten bevatten foutinjectie
            tag_error = tag_error_types[i % len(tag_error_types)]
            nmea_error = nmea_error_types[i % len(nmea_error_types)]

        tag_block = generate_tag_block(tag_error, i)
        nmea_sentence = generate_nmea_sentence(nmea_error)
        
        # Combineer TAG-blok en NMEA-zin
        full_message = "{}{}".format(tag_block, nmea_sentence)
        messages.append((i, tag_error, nmea_error, full_message))

    return messages

if __name__ == "__main__":
    # Pas de seriële poort aan indien nodig
    try:
        while True:
            test_messages = generate_test_messages()
            for counter, tag_error, nmea_error, message in test_messages:
                # Verstuur alleen als de seriële poort beschikbaar is
                try:
                    print("Bericht {}: TAG-fout - {}, NMEA-fout - {}".format(counter + 1, tag_error, nmea_error))
                    print(message)
                    print("Lengte: {} tekens\n".format(len(message.strip())))
                    s1.write(message)  # Verstuur het bericht
                    time.sleep(1)  # Wacht 1 seconde voor het volgende bericht
                except Exception as e:
                    print("Fout bij het verzenden van het bericht: {}".format(e))
    except Exception as e:
        print("Fout bij het openen van de seriële poort: {}".format(e))
