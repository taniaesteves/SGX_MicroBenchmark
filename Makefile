APP_DIR=App
DRIVER_OPENSSL_DIR=Openssl

Includes := -I$(APP_DIR) -I$(DRIVER_OPENSSL_DIR)
Link_Flags := -lpthread -lcrypto

CFLAGS := -Wall -pedantic $(Includes)


all: microbench_u_openssl

microbench_u_openssl: $(APP_DIR)/App.o $(APP_DIR)/openssl_impl.o $(DRIVER_OPENSSL_DIR)/symmetric.o
	gcc $^ -o $@ $(Link_Flags)

$(APP_DIR)/%.o: $(APP_DIR)/%.c
	gcc $(CFLAGS) -c $<  -o $@ 

$(DRIVER_OPENSSL_DIR)/%.o: $(DRIVER_OPENSSL_DIR)/%.c
	gcc $(CFLAGS) -c $<  -o $@ 

clean:
	rm -rf $(APP_DIR)/*.o $(DRIVER_OPENSSL_DIR)/*.o
	rm -rf microbench_u_openssl
