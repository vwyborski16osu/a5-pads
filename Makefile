all: keygen enc_client enc_server dec_client dec_server

enc_client:
	gcc -std=gnu99 -Wall -g -o enc_client enc_client.c

enc_server:
	gcc -std=gnu99 -Wall -g -o enc_server enc_server.c

dec_client:
	gcc -std=gnu99 -Wall -g -o dec_client dec_client.c

dec_server:
	gcc -std=gnu99 -Wall -g -o dec_server dec_server.c

keygen:
	gcc -std=gnu99 -Wall -g -o keygen keygen.c

clean:
	rm -f keygen enc_client enc_server dec_client dec_server