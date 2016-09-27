import socket as server


porta = 8802
HOST = '' 
ip = 'aaaa::0212:4B00:07C3:B5BB'

print('oi servidor')

servidor = server.socket (server.AF_INET6 , server.SOCK_DGRAM )

servidor.bind((HOST, porta))

dados = str()

print('servidor iniciado... porta:', porta)

mensagem = bytes(0x7A)

servidor.sendto(mensagem, (ip, porta))


while True:
    dados, endereco = servidor.recvfrom(porta)
    print("dados recebidos:", str(dados), "endereco", endereco[0])
    print("byte inicial", hex(dados[0]))
    servidor.sendto(mensagem, (ip, porta))        
    
    
