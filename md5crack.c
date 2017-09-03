/*
 * Simple MD5 implementation
 *
 * Compile with: gcc -o md5 -O3 -lm md5.c
 */
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stdint.h>
 #include <pthread.h>
 #include <unistd.h>
 
 // leftrotate function definition
 #define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))
 
 void md5(uint8_t *initial_msg, uint32_t *h0, uint32_t *h1, uint32_t *h2, uint32_t *h3) {
 
     size_t initial_len = 11;
 
     // Message (to prepare)
     uint8_t *msg = NULL;
 
     // Note: All variables are unsigned 32 bit and wrap modulo 2^32 when calculating
 
     // r specifies the per-round shift amounts
 
     uint32_t r[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                     5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
                     4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                     6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};
 
     // Use binary integer part of the sines of integers (in radians) as constants// Initialize variables:
     uint32_t k[] = {
             0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
             0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
             0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
             0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
             0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
             0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
             0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
             0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
             0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
             0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
             0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
             0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
             0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
             0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
             0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
             0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};
 
     *h0 = 0x67452301;
     *h1 = 0xefcdab89;
     *h2 = 0x98badcfe;
     *h3 = 0x10325476;
     int new_len = 56;
 
     msg = initial_msg;
     msg[initial_len] = 128; // write the "1" bit
 
     uint32_t bits_len = 8*initial_len; // note, we append the len
     memcpy(msg + new_len, &bits_len, 4);           // in bits at the end of the buffer
 
     int offset;
     for(offset=0; offset<new_len; offset += (512/8)) {
 
         // break chunk into sixteen 32-bit words w[j], 0 ≤ j ≤ 15
         uint32_t *w = (uint32_t *) (msg + offset);
 
 
         // Initialize hash value for this chunk:
         uint32_t a = *h0;
         uint32_t b = *h1;
         uint32_t c = *h2;
         uint32_t d = *h3;
 
         // Main loop:
         uint32_t i;
         for(i = 0; i<64; i++) {
             uint32_t f, g;
 
             if (i < 16) {
                 f = (b & c) | ((~b) & d);
                 g = i;
             } else if (i < 32) {
                 f = (d & b) | ((~d) & c);
                 g = (5*i + 1) % 16;
             } else if (i < 48) {
                 f = b ^ c ^ d;
                 g = (3*i + 5) % 16;
             } else {
                 f = c ^ (b | (~d));
                 g = (7*i) % 16;
             }
             uint32_t temp = d;
             d = c;
             c = b;
             b = b + LEFTROTATE((a + f + k[i] + w[g]), r[i]);
             a = temp;
 
 
 
         }
 
         // Add this chunk's hash to result so far:
 
         *h0 += a;
         *h1 += b;
         *h2 += c;
         *h3 += d;
 
     }
 }
 
 
 struct param
 {
     long start,end;
     int id;
 };
 
 volatile int running_threads = 0;
 pthread_mutex_t running_mutex = PTHREAD_MUTEX_INITIALIZER;
 
 int thread_num;
 long *status;
 struct param **pl;
 
 unsigned int bp[16];
 void *thread_func(void *pp)
 {
     struct param *p = pp;
     uint32_t h1,h2,h3,h0;
     long cnt = 0;
     long start = p->start;
     long end = p->end-1;
     int id = p->id;
     for(long i=start;i<end;i++)
     {
         status[id] = i;
         char buf[56+64];
         long num = i;
         for(int j=1;j<=10;j++)
         {
             buf[11-j] = num%10 + '0';
             num /= 10;
         }
         buf[0] = '1';
         md5(buf,&h0,&h1,&h2,&h3);
         uint8_t *p;
 
         p=(uint8_t *)&h0;
         //printf("%2.2x%2.2x%2.2x%2.2x", p[0], p[1], p[2], p[3], h0);
         if(!(bp[0]==p[0]&&bp[1]==p[1]&&bp[2]==p[2]&&bp[3]==p[3]))continue;
 
         p=(uint8_t *)&h1;
         //printf("%2.2x%2.2x%2.2x%2.2x", p[0], p[1], p[2], p[3], h1);
         if(!(bp[4]==p[0]&&bp[5]==p[1]&&bp[6]==p[2]&&bp[7]==p[3]))continue;
 
         p=(uint8_t *)&h2;
         //printf("%2.2x%2.2x%2.2x%2.2x", p[0], p[1], p[2], p[3], h2);
         if(!(bp[8]==p[0]&&bp[9]==p[1]&&bp[10]==p[2]&&bp[11]==p[3]))continue;
 
         p=(uint8_t *)&h3;
         //printf("%2.2x%2.2x%2.2x%2.2x", p[0], p[1], p[2], p[3], h3);
         if(!(bp[12]==p[0]&&bp[13]==p[1]&&bp[14]==p[2]&&bp[15]==p[3]))continue;
         printf("Found: %li\n",10000000000+i);
         exit(0);
     }
     pthread_mutex_lock(&running_mutex);
     running_threads--;
     pthread_mutex_unlock(&running_mutex);
     return NULL;
 }
 
 int main(int argc, char **argv) {
    printf("Target MD5:");
    scanf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
            ,&bp[0],&bp[1],&bp[2],&bp[3],&bp[4],&bp[5],&bp[6],&bp[7],&bp[8],&bp[9],&bp[10],&bp[11],&bp[12],&bp[13],&bp[14],&bp[15]);
    printf("Thread:");
    scanf("%i",&thread_num);
    pl = malloc(sizeof(struct param)*thread_num);
    status = malloc(sizeof(long)*thread_num);

    long per = (10000000000-1)/thread_num;
    for (int i = 0; i < thread_num;i++)
    {
        pthread_mutex_lock(&running_mutex);
        running_threads++;
        pthread_mutex_unlock(&running_mutex);
        // launch thread
        struct param *p = malloc(sizeof(struct param));
        pl[i] = p;
        p->start = per*i;
        if(i == thread_num-1)
        {
            p->end = 10000000000;
        }
        else
        {
            p->end = per*(i+1);
        }
        p->id = i;
        printf("thread[%i]: creating\n",i);
        status[i] = 0;
        pthread_t t;
        pthread_create(&t,NULL, thread_func, p);
    }
 
    puts("");
    long double last_tsted = 0;
    int a=0,b=0,c=0;
    do
    {
    long double tested = 0;
        printf("\033[%iA",thread_num+1);
        for (int i = 0; i < thread_num;i++)
        {
            struct param *p = pl[i];
            printf("\033[Kthread[%i]: %li/%li\r\n",i,10000000000+status[i],10000000000+p->end);
            tested+=status[i]-p->start;
        }
        tested = tested>0?tested:0;
        int hrs=0;
        int min=0;
        int sec=0;
        int totalsec;
        long double qps = tested-last_tsted;
        totalsec = 9999999999/qps;
        hrs = totalsec/60/60;
        min = totalsec/60%60;
        sec = totalsec%60;
        c++;
        if(c>60){b++;c=0;}
        if(b>60){a++;b=0;}
        printf("\033[K%LF%% done. %.LFkqps. %.2i:%.2i:%.2i used. %.2i:%.2i:%.2i remains.\n",tested/9999999999*100,(qps)/1000,a,b,c,hrs,min,sec);
        last_tsted = tested;
        sleep(1);
    }
    while (running_threads > 0);

    puts("Not found");
 }
 
 
