import socket
import time

# --- Configuration ---
HOST = '127.0.0.1'
PORT = 9999
BUFFER_SIZE = 1024
QUIT_COMMAND = 'quit' 

# Format: (Question, [Option1, Option2, ...], Correct_Option_Index)
QUIZ_QUESTIONS = [
    ("What is the capital of France?", ["Paris", "Rome", "Madrid"], 0),
    ("What is 5 + 7?", ["10", "12", "15"], 1),
    ("Which planet is known as the Red Planet?", ["Mars", "Jupiter", "Venus"], 0),
    ("How many colours are there in a rainbow?", ["5", "6", "7"], 2),
    ("What is the largest ocean on Earth?", ["Atlantic", "Indian", "Pacific"], 2),
    ("What data structure does the LIFO principle follow?", ["Queue", "Stack", "Array"], 1)
]

client_states = {}
SERVER_RUNNING = True 

def get_question_string(q_index):
    """Formats a question for the client."""
    q, options, _ = QUIZ_QUESTIONS[q_index]
    options_str = "\n".join([f"{i+1}. {opt}" for i, opt in enumerate(options)])
    return f"Question {q_index + 1}/{len(QUIZ_QUESTIONS)}: {q}\nOptions:\n{options_str}\nEnter your answer number (1, 2, or 3):"

def process_client_message(data, addr):
    """Processes a message from a client and returns a response."""
    global SERVER_RUNNING 

    if addr not in client_states:
        client_states[addr] = {'score': 0, 'current_question': 0}
        print(f"New client connected: {addr}")
        
    state = client_states[addr]
    q_index = state['current_question']
    
    message = data.decode('utf-8').strip().lower()

    if message == QUIT_COMMAND:
        SERVER_RUNNING = False
        print(f"Server received '{QUIT_COMMAND}' from {addr}. Shutting down server.")
        return "Server received quit command. Shutting down."

    if message in ('start', 'restart'):
        state['score'] = 0
        state['current_question'] = 0
        q_index = 0 
        return get_question_string(q_index) 

    if q_index >= len(QUIZ_QUESTIONS):
        return f"Quiz Finished! Your final score is {state['score']} out of {len(QUIZ_QUESTIONS)}.\n\nType 'restart' to play again or '{QUIT_COMMAND}' to exit."

    if message.isdigit():
        try:
            answer = int(message) - 1
            
            _, options, correct_answer_index = QUIZ_QUESTIONS[q_index]
            
            if answer == correct_answer_index:
                state['score'] += 1
                feedback = "**Correct!** Your current score is "
            else:
                correct_option = options[correct_answer_index]
                feedback = f"**Incorrect.** The correct answer was **{correct_option}**. Your current score is "
                
            state['current_question'] += 1
            
            state_after_answer = client_states[addr]
            next_q_index = state_after_answer['current_question']
            
            if next_q_index < len(QUIZ_QUESTIONS):
                next_question = get_question_string(next_q_index)
                return f"{feedback}{state_after_answer['score']}.\n\n{next_question}"
            else:
                final_message = f"Quiz Finished! Final Score: {state_after_answer['score']} out of {len(QUIZ_QUESTIONS)}."
                return f"{final_message}\n\nType 'restart' to play again or '{QUIT_COMMAND}' to exit."

        except (ValueError, IndexError):
            return "Invalid input. Please enter the number (1, 2, or 3) of your chosen option."
    
    if q_index == 0:
        return f"Welcome to the UDP Quiz! Type 'start' to begin or '{QUIT_COMMAND}' to exit."
        
    return f"Invalid command or answer format. Please try again or type 'restart'."

def run_server():
    global SERVER_RUNNING
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_socket.bind((HOST, PORT))
    server_socket.settimeout(1.0) 
    print(f"UDP Quiz Server listening on {HOST}:{PORT}")

    try:
        while SERVER_RUNNING:
            try:
                data, addr = server_socket.recvfrom(BUFFER_SIZE)
                
                response = process_client_message(data, addr)
                
                if SERVER_RUNNING:
                    server_socket.sendto(response.encode('utf-8'), addr)
                else:
                    server_socket.sendto(response.encode('utf-8'), addr) 
                
                print(f"[{time.strftime('%H:%M:%S')}] Message from {addr}: {data.decode('utf-8').strip()}")
                
            except socket.timeout:
                continue 
            except KeyboardInterrupt:
                SERVER_RUNNING = False
                
    finally:
        print("\nServer shutting down.")
        server_socket.close()

if __name__ == '__main__':
    run_server()