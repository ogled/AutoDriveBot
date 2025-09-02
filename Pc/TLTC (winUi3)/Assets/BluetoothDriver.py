import sys
import asyncio
from bleak import BleakClient

# UUIDs
VEX_DATA_RX_CHARACTERISTIC_UUID = "08590f7e-db05-467e-8757-72f6faeb1326"
VEX_DATA_TX_CHARACTERISTIC_UUID = "08590F7E-DB05-467e-8757-72F6FAEB1306"
VEX_DATA_TX2_CHARACTERISTIC_UUID = "08590F7E-DB05-467e-8757-72F6FAEB1316"
VEX_CONTROL_CHARACTERISTIC_UUID = "08590f7e-db05-467e-8757-72f6faeb13e5"

PIPE_TX = r'\\.\pipe\AutoDriveBotTx'
PIPE_RX = r'\\.\pipe\AutoDriveBotRx'


class VEXRobotController:
	def __init__(self, mac_address, pipe_tx, pipe_rx):
		self.mac_address = mac_address
		self.pipe_tx = pipe_tx
		self.pipe_rx = pipe_rx
		self.pipe_out = None
		self.pipe_in = None
		self.conn_code = None
		self.buffer = ""
		self.received_data = asyncio.Queue()

	async def open_pipes(self):
		self.pipe_out = open(self.pipe_rx, 'w', encoding='utf-8', buffering=1)
		self.pipe_in = open(self.pipe_tx, 'r', encoding='utf-8')

	async def close_pipes(self):
		if self.pipe_out:
			self.pipe_out.close()
		if self.pipe_in:
			self.pipe_in.close()

	async def write_pipe(self, msg: str):
		try:
			self.pipe_out.write(msg + "\n")
			self.pipe_out.flush()
		except Exception:
			pass

	async def read_pipe(self):
		try:
			line = await asyncio.to_thread(self.pipe_in.readline)
			return line.strip()
		except Exception:
			return ""

	async def notification_handler(self, sender: int, data: bytearray):
		"""Обрабатывает входящие уведомления"""
		try:
			text = data.decode('utf-8', errors='replace')
			self.buffer += text
			while "\r\n" in self.buffer:
				msg, self.buffer = self.buffer.split("\r\n", 1)
				if msg:
					await self.received_data.put(msg)
		except Exception as e:
			await self.write_pipe(f"ERROR: Notification handler failed: {e}")

	async def print_received_data(self):
		while True:
			data = await self.received_data.get()
			await self.write_pipe(f"RECEIVED: {data}")
			self.received_data.task_done()

	async def do_handshake(self, client: BleakClient) -> bool:
		"""Процедура подключения с deadface + кодом"""
		# deadface check
		value = await client.read_gatt_char(VEX_CONTROL_CHARACTERISTIC_UUID)
		if value != b'\xde\xad\xfa\xce':
			await self.write_pipe("ERROR: Deadface handshake failed")
			return False

		# отправляем ff ff ff ff
		await client.write_gatt_char(VEX_CONTROL_CHARACTERISTIC_UUID, b'\xff\xff\xff\xff', response=False)
		await asyncio.sleep(0.5)

		# получаем код от пользователя (если ещё не знаем)
		if self.conn_code is None:
			await self.write_pipe("REQUEST: Enter connection code")
			conn_code_str = await self.read_pipe()
			self.conn_code = int(conn_code_str)

		conn_bytes = bytearray([
			(self.conn_code // 1000) % 10,
			(self.conn_code // 100) % 10,
			(self.conn_code // 10) % 10,
			self.conn_code % 10
		])

		await client.write_gatt_char(VEX_CONTROL_CHARACTERISTIC_UUID, conn_bytes, response=False)

		# проверка
		value = await client.read_gatt_char(VEX_CONTROL_CHARACTERISTIC_UUID)
		if value != conn_bytes:
			await self.write_pipe("ERROR: Connection code verification failed")
			return False

		return True

	async def connect(self) -> BleakClient | None:
		client = BleakClient(self.mac_address, timeout=5.0)
		try:
			await self.write_pipe("STATUS: Connecting...")
			await client.connect()

			if not await self.do_handshake(client):
				await client.disconnect()
				return None

			# Подписка на уведомления
			await client.start_notify(VEX_DATA_TX_CHARACTERISTIC_UUID, self.notification_handler)
			await client.start_notify(VEX_DATA_TX2_CHARACTERISTIC_UUID, self.notification_handler)

			await self.write_pipe("STATUS: Connected successfully")
			return client

		except asyncio.TimeoutError:
			await self.write_pipe("ERROR: Connection timed out")
		except Exception as e:
			await self.write_pipe(f"ERROR: Connection failed: {e}")
		return None

	async def send_data(self, client: BleakClient, data: str):
		try:
			if not data.endswith("\r\n"):
				data += "\r\n"
			await client.write_gatt_char(VEX_DATA_RX_CHARACTERISTIC_UUID, data.encode('utf-8'), response=False)
		except Exception as e:
			await self.write_pipe(f"ERROR: Send failed: {e}")


async def main(mac_address):
	controller = VEXRobotController(mac_address, PIPE_TX, PIPE_RX)
	await controller.open_pipes()
	print_task = asyncio.create_task(controller.print_received_data())

	try:
		while True:
			client = await controller.connect()
			if not client:
				await asyncio.sleep(2)
				continue

			try:
				while client.is_connected:
					cmd = await controller.read_pipe()
					if cmd:
						if cmd.lower() in ("quit", "exit"):
							return
						await controller.send_data(client, cmd)
			except Exception:
				pass

			await client.disconnect()
			await controller.write_pipe("STATUS: Disconnected")
			await asyncio.sleep(2)

	finally:
		await controller.close_pipes()
		print_task.cancel()
		try:
			await print_task
		except asyncio.CancelledError:
			pass


if __name__ == "__main__":
	if len(sys.argv) != 2:
		print("Usage: python script.py <MAC_ADDRESS>")
		sys.exit(1)
	asyncio.run(main(sys.argv[1]))
