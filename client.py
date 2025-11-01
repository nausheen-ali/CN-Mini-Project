import socket

# --- Configuration ---
HOST = '127.0.0.1'  # Server's IP address
PORT = 9999        # Server's port
BUFFER_SIZE = 1024
QUIT_COMMAND = 'quit' 

def run_client():
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    client_socket.settimeout(5.0)

    print("Welcome to the UDP Quiz Client.")
    print(f"Type 'start' to begin, '{QUIT_COMMAND}' to exit.")

    server_address = (HOST, PORT)
    
    try:
        initial_message = "start"
        client_socket.sendto(initial_message.encode('utf-8'), server_address)
        
        while True:
            try:
                print("\nWaiting for server response...")
                data, server = client_socket.recvfrom(BUFFER_SIZE)
                
                server_response = data.decode('utf-8')
                print("-" * 40)
                print(server_response)
                print("-" * 40)
            
            except socket.timeout:
                print("Connection timed out. Did the server shut down?")
                break 
            
            except Exception as e:
                print(f"An error occurred: {e}")
                break

            if "Server received quit command. Shutting down." in server_response:
                print("Server has shut down. Exiting client.")
                break

            user_input = input("Your input: ")

            if user_input.lower() == QUIT_COMMAND:
                print("Sending quit command to server and exiting client.")
                client_socket.sendto(user_input.encode('utf-8'), server_address)
                break
            
            client_socket.sendto(user_input.encode('utf-8'), server_address)
            
    except KeyboardInterrupt:
        print("\nClient interrupted. Exiting.")
    except ConnectionRefusedError:
        print("Connection failed. Ensure the server is running.")
    finally:
        client_socket.close()

if __name__ == '__main__':
    run_client()