import asyncio
import sys
from bleak import BleakClient

# UUIDs
VEX_DATA_RX_CHARACTERISTIC_UUID = "08590f7e-db05-467e-8757-72f6faeb1326"
VEX_DATA_TX_CHARACTERISTIC_UUID = "08590F7E-DB05-467E-8757-72F6FAEB1306"
VEX_DATA_TX2_CHARACTERISTIC_UUID = "08590F7E-DB05-467E-8757-72F6FAEB1316"
VEX_CONTROL_CHARACTERISTIC_UUID = "08590f7e-db05-467e-8757-72f6faeb13e5"

PIPE_TX = r'\\.\pipe\AutoDriveBotTx'
PIPE_RX = r'\\.\pipe\AutoDriveBotRx'

class VEXRobotController:
    def __init__(self, mac_address, pipe_tx, pipe_rx):
        self.client = BleakClient(mac_address)
        self.buffer = ""
        self.received_data = asyncio.Queue()
        self.pipe_tx = pipe_tx
        self.pipe_rx = pipe_rx
        self.pipe_out = None
        self.pipe_in = None

    async def notification_handler(self, sender, data):
        try:
            data_str = data.decode('utf-8', errors='replace')
            self.buffer += data_str
            while "\r\n" in self.buffer:
                msg, self.buffer = self.buffer.split("\r\n", 1)
                if msg:
                    await self.received_data.put(msg)
        except Exception as e:
            await self.write_pipe(f"ERROR: Notification handler failed: {e}")

    async def open_pipes(self):
        self.pipe_out = open(self.pipe_rx, 'w', encoding='utf-8', buffering=1)
        self.pipe_in = open(self.pipe_tx, 'r', encoding='utf-8')

    async def close_pipes(self):
        if self.pipe_out:
            self.pipe_out.close()
        if self.pipe_in:
            self.pipe_in.close()

    async def write_pipe(self, message):
        try:
            self.pipe_out.write(message + "\n")
            self.pipe_out.flush()
        except Exception:
            pass

    async def read_pipe(self):
        try:
            # Используем неблокирующее чтение
            line = await asyncio.to_thread(self.pipe_in.readline)
            return line.strip()
        except Exception:
            return ""

    async def print_received_data(self):
        while True:
            data = await self.received_data.get()
            await self.write_pipe(f"RECEIVED: {data}")
            self.received_data.task_done()

    async def connect(self):
        try:
            await self.open_pipes()
            await self.write_pipe("STATUS: Connecting...")
            await self.client.connect()

            value = await self.client.read_gatt_char(VEX_CONTROL_CHARACTERISTIC_UUID)
            if value != b'\xde\xad\xfa\xce':
                await self.write_pipe("ERROR: Deadface handshake failed")
                return False

            await self.client.write_gatt_char(VEX_CONTROL_CHARACTERISTIC_UUID, b'\xff\xff\xff\xff')
            await self.write_pipe("REQUEST: Enter connection code")

            conn_code_str = await self.read_pipe()
            conn_code = int(conn_code_str)
            conn_bytes = bytearray([
                (conn_code // 1000) % 10,
                (conn_code // 100) % 10,
                (conn_code // 10) % 10,
                conn_code % 10
            ])

            await self.client.write_gatt_char(VEX_CONTROL_CHARACTERISTIC_UUID, conn_bytes)
            value = await self.client.read_gatt_char(VEX_CONTROL_CHARACTERISTIC_UUID)
            if value != conn_bytes:
                await self.write_pipe("ERROR: Connection code verification failed")
                return False

            await self.client.start_notify(VEX_DATA_TX_CHARACTERISTIC_UUID, self.notification_handler)
            await self.client.start_notify(VEX_DATA_TX2_CHARACTERISTIC_UUID, self.notification_handler)
            await self.write_pipe("STATUS: Connected successfully")

            # Триггерим устройство, если нужно
            await self.send_data("HELLO")

            return True
        except Exception as e:
            await self.write_pipe(f"ERROR: Connection failed: {e}")
            return False

    async def send_data(self, data):
        try:
            if not isinstance(data, str):
                data = str(data)
            if not data.endswith("\r\n"):
                data += "\r\n"
            await self.client.write_gatt_char(VEX_DATA_RX_CHARACTERISTIC_UUID, data.encode('utf-8'))
            await self.write_pipe(f"SENT: {data.strip()}")
        except Exception as e:
            await self.write_pipe(f"ERROR: Send failed: {e}")

    async def disconnect(self):
        try:
            if self.client.is_connected:
                await self.client.stop_notify(VEX_DATA_TX_CHARACTERISTIC_UUID)
                await self.client.stop_notify(VEX_DATA_TX2_CHARACTERISTIC_UUID)
                await self.client.disconnect()
            await self.close_pipes()
            await self.write_pipe("STATUS: Disconnected")
        except Exception as e:
            await self.write_pipe(f"ERROR: Disconnect failed: {e}")

async def main(mac_address):
    controller = VEXRobotController(mac_address, PIPE_TX, PIPE_RX)
    print_task = asyncio.create_task(controller.print_received_data())

    try:
        if not await controller.connect():
            return

        while True:
            try:
                user_input = await controller.read_pipe()
                if not user_input:
                    await asyncio.sleep(0.1)
                    continue

                if user_input.lower() in ('quit', 'exit'):
                    break

                await controller.send_data(user_input)

            except (KeyboardInterrupt, EOFError):
                break
    finally:
        await controller.disconnect()
        print_task.cancel()
        try:
            await print_task
        except asyncio.CancelledError:
            pass

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <MAC_ADDRESS>")
        sys.exit(1)

    mac_address = sys.argv[1]
    try:
        asyncio.run(main(mac_address))
    except KeyboardInterrupt:
        pass
