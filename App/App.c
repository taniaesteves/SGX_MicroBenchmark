#include "App.h"

#define CLIENT_KEY_SIZE 16
#define IV_SIZE 16
#define HASH_LEN 32 

char CLIENT_KEY[16]  = "C53C0E2F1B0B19A";
unsigned char IV[16] = "C53C0E2F1B0B19A";

void print_digest(unsigned char *digest, int digest_size) {
    int n;
	char mdString[(digest_size * 2) + 1];
	for (n = 0; n < digest_size; ++n) {
        snprintf(&(mdString[n*2]), digest_size*2, "%02x", (unsigned int)digest[n]);
    }
    
    printf("\'%s\' -> ", mdString);
}

void randstring(unsigned char *dest, size_t length) {

    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";        
    unsigned char *randomString = NULL;
    size_t n;

    if (length) {
        randomString = (unsigned char*) malloc(sizeof(unsigned char) * length);

        if (randomString) {            
            for (n = 0;n < length-1;n++) {            
                int key = rand() % (int)(sizeof(charset) -1);
                randomString[n] = charset[key];
            }

            randomString[length] = '\0';
        }
    }
    memcpy(dest, randomString, length);
    free(randomString);
}

/* TEST1: Compute hash and reencrypt data */
double func_test1(uint8_t *dest, size_t dest_len, uint8_t *str, size_t str_len, uint8_t *hash) {
    clock_t enclave_startTime, enclave_endTime;
    enclave_startTime = clock();
    reencrypt_hash(IV, IV_SIZE, dest, dest_len, str, str_len, hash, HASH_LEN);
    enclave_endTime = clock();
    
    return (double)(enclave_endTime - enclave_startTime) / CLOCKS_PER_SEC;
}

/* TEST2: Reencrypt data */
double func_test2(uint8_t *dest, size_t dest_len, uint8_t *str, size_t str_len, uint8_t *hash) {
    clock_t enclave_startTime, enclave_endTime;
    enclave_startTime = clock();
    reencrypt( IV, IV_SIZE, dest, dest_len, str, str_len);
    enclave_endTime = clock();
    
    return (double)(enclave_endTime - enclave_startTime) / CLOCKS_PER_SEC;
}

/* TEST3: Compute hash */
double func_test3(uint8_t *dest, size_t dest_len, uint8_t *str, size_t str_len, uint8_t *hash) {
    clock_t enclave_startTime, enclave_endTime;
    enclave_startTime = clock();
    if (compute_hash(str, str_len, hash, HASH_LEN) != EXIT_SUCCESS){
        printf("Error computing hash\n");
        exit(EXIT_FAILURE);
    }
    enclave_endTime = clock();
    return (double)(enclave_endTime - enclave_startTime) / CLOCKS_PER_SEC;
}

void run_test_ops(double (*func_test)(uint8_t*, size_t, uint8_t*, size_t, uint8_t*), uint8_t *dest, size_t dest_len, uint8_t *str, size_t str_len, uint64_t n_ops) {
    double total_enclave_time_elapsed=0;
    uint64_t ops = 0;

    unsigned char *hash = (unsigned char*) malloc (sizeof(unsigned char) * HASH_LEN);

	while (ops < n_ops) 
	{
		// call test func
        total_enclave_time_elapsed += func_test(dest, dest_len, str, str_len, hash);
        ops++;
	}

    free(hash);
    
    // print results
    printf("\n\nTotal ops:  %lu\n", ops);
    printf("Throughput: %.3f ops/second\n", (double) ops / total_enclave_time_elapsed);
    printf("Latency:    %.3f miliseconds\n", (total_enclave_time_elapsed / ops) * 1000);
    printf("Total time: %f \n", total_enclave_time_elapsed);           
}

void run_test_time(double (*func_test)(uint8_t*, size_t, uint8_t*, size_t, uint8_t*), uint8_t *dest, size_t dest_len, uint8_t *str, size_t str_len, uint8_t time_to_run) {
	unsigned int x_seconds=0, x_milliseconds=0;
	unsigned int count_down_time_in_secs, time_left=0;
	double total_enclave_time_elapsed=0;
    clock_t x_startTime, x_countTime;
    uint64_t ops = 0;

    unsigned char *hash = (unsigned char*) malloc (sizeof(unsigned char) * HASH_LEN);

    count_down_time_in_secs = time_to_run * 60;
    x_startTime=clock();  // start clock
    time_left=count_down_time_in_secs-x_seconds;   // update timer
	while (time_left>0) {
        // call test func
        total_enclave_time_elapsed += func_test(dest, dest_len, str, str_len, hash);    
        ops++;
		
        // update time_left
		x_countTime=clock(); 
        x_milliseconds=x_countTime-x_startTime;
        x_seconds=(x_milliseconds/(CLOCKS_PER_SEC));
        time_left=count_down_time_in_secs-x_seconds;
		// printf("%.2d\n", time_left);
        // printf("ops: %llu\n", ops);
	}

    free(hash);
    
    // print results
    printf("\n\nTotal ops:  %lu\n", ops);
    printf("Throughput: %.3f ops/second\n", (double) ops / total_enclave_time_elapsed);
    printf("Latency:    %.3f miliseconds\n", (total_enclave_time_elapsed / ops) * 1000);
    printf("Total time: %f \n", total_enclave_time_elapsed);           
}

void usage(void){
	printf(" Help:\n\n");
    printf(" -r<value> Number of test (1:hash+encrypt, 2:encrypt, 3:hash)\n");
	printf(" -b<value> block size (default=128)\n");
	printf(" -t<value> or -n<value>\t(Benchmark duration (-t) in Minutes or number of operations to execute (-n))\n");
	exit (8);
}

int main(int argc, char const *argv[]) {

    int test=3, ciphertext_size;
    size_t block_size=128;
    unsigned int time_to_run=0;
    uint64_t n_ops=0;

    while ((argc > 1) && (argv[1][0] == '-')) {
		switch (argv[1][1]) {
			case 'r':
				test=atoi(&argv[1][2]);
				break;
			case 'b':
				block_size=atoi(&argv[1][2]);
				break;
			case 't':
                if (n_ops != 0) { printf("Cannot use both -t and -n\n\n"); usage(); }
				else
                    time_to_run=atoi(&argv[1][2]);
				break;
            case 'n':
                if (time_to_run != 0) { printf("Cannot use both -t and -n\n\n"); usage(); }
				else
                    n_ops=atoi(&argv[1][2]);
				break;
			case 'h':
				usage();
				break;
			default:
				printf("Wrong Argument: %s\n", argv[1]);
				usage();
				exit(0);
				break;
        }

		++argv;
		--argc;
	}

    unsigned char *randomstr  = (unsigned char*) malloc(sizeof(unsigned char) * block_size);
    unsigned char *ciphertext = (unsigned char*) malloc(sizeof(unsigned char) * (block_size + 16) );
    unsigned char *dest = (unsigned char*) malloc(sizeof(unsigned char) * (block_size + 16) );

    // Generate a random string with size = block_size
    randstring(randomstr, block_size);

    // Encrypt random string
    if (openssl_init(CLIENT_KEY, CLIENT_KEY_SIZE) != EXIT_SUCCESS) printf("<T> openssl init error!\n");
    ciphertext_size = openssl_encode(IV, ciphertext, randomstr, block_size);

    if (n_ops > 0) printf("Running test %d with block_size = %ldB and n_ops = %lu\n", test, block_size, n_ops);
    else printf("Running test %d with block_size = %ldB and time_to_run = %um\n", test, block_size, time_to_run);
    // printf("press Enter to start...\n");
    // getchar();

    switch(test) {
        case 1:     
            init_u_openssl(CLIENT_KEY, CLIENT_KEY_SIZE);       
            if (n_ops > 0) run_test_ops(func_test1, dest, ciphertext_size, ciphertext, ciphertext_size, n_ops);
            else run_test_time(func_test1, dest, ciphertext_size, ciphertext, ciphertext_size, time_to_run);
            break;
        case 2:            
            init_u_openssl(CLIENT_KEY, CLIENT_KEY_SIZE);
            if (n_ops > 0) run_test_ops(func_test2, dest, ciphertext_size, ciphertext, ciphertext_size, n_ops);
            else run_test_time(func_test2, dest, ciphertext_size, ciphertext, ciphertext_size, time_to_run);
            break;
        case 3:
            if (n_ops > 0) run_test_ops(func_test3, dest, ciphertext_size, randomstr, block_size, n_ops);
            else run_test_time(func_test3, dest, ciphertext_size, randomstr, block_size, time_to_run);
            break;
    }

    free(randomstr);
    free(ciphertext);
    free(dest);
    return EXIT_SUCCESS;
}