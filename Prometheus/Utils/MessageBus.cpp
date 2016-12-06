#include "MessageBus.h"

namespace Prometheus {

	std::vector<MessageBus::Message> MessageBus::MessageStack;
	std::vector<unsigned int> MessageBus::CleanEntries;

	void MessageBus::pushMessage(Message msg, bool priority) {
		if (priority) {
			std::vector<Message>::iterator i;
			i = MessageStack.begin();
			MessageStack.insert(i, msg);
			return;
		}else if (CleanEntries.size() > 0) {
			unsigned int entry = CleanEntries.back();
			MessageStack[entry] = msg;
			CleanEntries.pop_back();
		} else {
			MessageStack.push_back(msg);
		}
	}

	void MessageBus::removeMessage(unsigned int position) {
		MessageStack[position] = Message();
		CleanEntries.push_back(position);
	}

	void MessageBus::CleanStack() {
		for (unsigned int i = 0; i < MessageStack.size(); i++) {
			if (MessageStack[i].read == true) {
				removeMessage(i);
			}
		}
	}

	std::vector<MessageBus::Message> MessageBus::getStack() {
		return MessageStack;
	}

}