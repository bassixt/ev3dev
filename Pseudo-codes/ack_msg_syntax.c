		//send ACK msg

			*((uint16_t *) ack) = msgId++;
			ack[2] = TEAM_ID;
			ack[3] = next;
			ack[4] = MSG_ACK;
			ack[5] = string[0]; // Id ack 
			ack[6] = string[1]; // Id ack
			ack[7] = 0; // 0 if it OK, 1 if it failed 
			write(s, ack, 8);