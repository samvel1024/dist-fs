#include "Alarm.h"
#include <chrono>
#include "Error.h"

using namespace std::chrono;


Alarm::Alarm(uint64_t millis, std::function<void()> callback) : callback(std::move(callback)) {
	if (millis <= 0) {
		throw Error("Illegal millis value");
	}
	uint64_t now = current_time_millis();
	this->millis = now + millis;
}

uint64_t Alarm::get_timeout_time() {
	return millis;
}

const std::function<void()> &Alarm::get_callback() const {
	return callback;
}

uint64_t current_time_millis() {
	milliseconds ms = duration_cast<milliseconds>(
		system_clock::now().time_since_epoch()
	);
	return ms.count();
}
