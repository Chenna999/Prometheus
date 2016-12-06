#pragma once

#include <vector>

namespace Prometheus {

	class MessageBus {
	public:
		enum MessageType { NO_MESSAGE = 0,
						   WINDOW_SHOULD_CLOSE
						 };

		struct Message {
			MessageType type = NO_MESSAGE;
			void* data = nullptr;
			bool read = false;
		};

		static std::vector<Message> getStack();
		static void pushMessage(Message msg, bool priority = false);
		static void removeMessage(unsigned int position);
		static void CleanStack();

	private:
		static std::vector<Message> MessageStack;
		static std::vector<unsigned int> CleanEntries;
	};

}