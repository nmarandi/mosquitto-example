#include <mosquitto.h>
#include <stdexcept>
#include <string>
#include <iostream>
#include <thread>

#define CLIENT_ID "Client_ID"
#define SERVER_ID "SERVER_ID"
#define BROKER_ADDRESS "localhost"
#define MQTT_PORT 1883
#define MQTT_TOPIC "test/topic"

using std::chrono_literals::operator ""s;

void mosq_publisher()
{
	try {
		int res;
		auto mosquitto = mosquitto_new(SERVER_ID, true, nullptr);
		res = mosquitto_connect(mosquitto, BROKER_ADDRESS, MQTT_PORT, 0);
		if (res)
			throw std::runtime_error("mosquitto connect problem = " + std::to_string(res));
		const auto mess = "hello world";
		for (int i = 0; i<10;i++) {
			res = mosquitto_publish(mosquitto, nullptr, MQTT_TOPIC, strlen(mess), mess, 0, false);
			if (res)
				throw std::runtime_error("mosquitto publish problem = " + std::to_string(res));
			std::this_thread::sleep_for(2s);
		}
		res = mosquitto_disconnect(mosquitto);
		mosquitto_destroy(mosquitto);
	}
	catch (std::exception& e)
	{
		std::cout << e.what();
	}
}

void on_message(struct mosquitto *, void *, const struct mosquitto_message * message)
{
	std::cout << "message = " << std::string(static_cast<char*>(message->payload), message->payloadlen) << std::endl;
}

void mosq_subscriber()
{
	try {
		int res ;
		auto mosquitto = mosquitto_new(CLIENT_ID, true, nullptr);
		res = mosquitto_connect(mosquitto, BROKER_ADDRESS, MQTT_PORT, 0);
		if (res)
			throw std::runtime_error("mosquitto connect problem = " + std::to_string(res));
		res = mosquitto_subscribe(mosquitto, nullptr, MQTT_TOPIC, 0);
		mosquitto_message_callback_set(mosquitto, on_message);
		for (int i = 0; i<25; i++) {
			res = mosquitto_loop(mosquitto, 1000, 1);
			std::this_thread::sleep_for(1s);
		}
		if (res)
			throw std::runtime_error("mosquitto start loop problem = " + std::to_string(res));
		res = mosquitto_disconnect(mosquitto);
		mosquitto_destroy(mosquitto);
	}
	catch (std::exception& e)
	{
		std::cout << e.what();
	}
}

int main(int argc, char *argv[])
{
	auto res = mosquitto_lib_init();
	if (res)
		throw std::runtime_error("mosquitto lib init problem");
	std::thread sub(mosq_subscriber);
	std::this_thread::sleep_for(2s);
	std::thread pub(mosq_publisher);
	pub.join();
	sub.join();
	res = mosquitto_lib_cleanup();
	std::cout << "press any key to finish" << std::endl;
	getchar();
    return 0;
}

