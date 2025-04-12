import serial
import socket
import sys
import time


class NMEAMessage:
    def __init__(self, raw: str = None, talker: str = "", sentence_type: str = "", fields: list = None):
        """
        Initialize the NMEAMessage either by parsing a raw message or by constructing from components.
        """
        if raw:
            self.raw = raw.strip()
            self.parse_raw()
        else:
            if not (talker and sentence_type):
                raise ValueError("Talker and sentence_type must be provided when not parsing a raw message.")
            self.talker = talker
            self.sentence_type = sentence_type
            self.fields = fields if fields else []
            self.checksum = self.compute_checksum()
            self.raw = self.to_string()

    def parse_raw(self):
        """
        Parse the raw NMEA message into components.
        """
        if not self.raw.startswith("$"):
            raise ValueError("NMEA message must start with '$'")

        try:
            data_part, checksum_str = self.raw[1:].split("*")
        except ValueError:
            raise ValueError("NMEA message must contain a '*' followed by checksum.")

        self.checksum = checksum_str
        self.compute_checksum()  # To ensure the checksum is computed

        parts = data_part.split(",")
        header = parts[0]
        if len(header) < 5:
            raise ValueError("Invalid NMEA header.")

        self.talker = header[:2]
        self.sentence_type = header[2:]
        self.fields = parts[1:]

        if not self.validate_checksum():
            raise ValueError("Invalid checksum.")

    def compute_checksum(self) -> str:
        """
        Compute the checksum by XORing all characters between $ and *.
        """
        checksum = 0
        # Exclude the starting '$' and everything after '*'
        for char in self.raw[1 : self.raw.find("*")]:
            checksum ^= ord(char)
        self.computed_checksum = f"{checksum:02X}"
        return self.computed_checksum

    def validate_checksum(self) -> bool:
        """
        Validate the checksum of the NMEA message.
        """
        return self.computed_checksum.upper() == self.checksum.upper()

    def to_string(self) -> str:
        """
        Construct the raw NMEA sentence from components.
        """
        data = f"{self.talker}{self.sentence_type},{','.join(self.fields)}"
        checksum = self.compute_checksum_static(data)
        return f"${data}*{checksum}"

    @staticmethod
    def compute_checksum_static(data: str) -> str:
        """
        Static method to compute checksum from data string.
        """
        checksum = 0
        for char in data:
            checksum ^= ord(char)
        return f"{checksum:02X}"

    def send(self, method: str = "serial", **kwargs):
        """
        Send the NMEA message via the specified method.

        - For 'serial', provide 'port' and 'baudrate'.
        - For 'socket', provide 'host' and 'port'.
        """
        message = self.to_string() + "\r\n"  # NMEA sentences typically end with CRLF

        if method == "serial":
            port = kwargs.get("port", "/dev/ttyUSB0" if sys.platform != "win32" else "COM3")
            baudrate = kwargs.get("baudrate", 4800)
            try:
                with serial.Serial(port, baudrate, timeout=1) as ser:
                    ser.write(message.encode("ascii"))
                print(f"Sent via serial: {message.strip()}")
            except serial.SerialException as e:
                print(f"Serial error: {e}")
        elif method == "socket":
            host = kwargs.get("host", "localhost")
            port = kwargs.get("port", 5000)
            try:
                with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
                    sock.connect((host, port))
                    sock.sendall(message.encode("ascii"))
                print(f"Sent via socket to {host}:{port}: {message.strip()}")
            except socket.error as e:
                print(f"Socket error: {e}")
        else:
            raise ValueError("Unsupported method. Use 'serial' or 'socket'.")


if __name__ == "__main__":
    # Example usage
    nmea = NMEAMessage(raw="$GPRMC,123456.00,A,1234.5678,N,12345.6789,W,0.0,0.0,010101,0.0,E*47")
    print(f"Parsed NMEA Message: {nmea.raw}")
    print(f"Talker: {nmea.talker}, Sentence Type: {nmea.sentence_type}, Fields: {nmea.fields}, Checksum: {nmea.checksum}")

    # Sending the message via serial (example)
    while True:
        nmea.send(method="serial", port="/dev/ttyUSB0", baudrate=4800)
        time.sleep(2)
