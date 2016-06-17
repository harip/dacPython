/*
 *
 
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/spidev0.1";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay;

#define READ_BIT	0x80

uint32_t v_ref = 2186;
uint16_t dac_bits = 16;
uint32_t resolution;

int32_t val_mv = 50;

static void stream(int fd)
{
	
	uint16_t D_val16;

	int ret;
	uint16_t rawval;
	int32_t slope, val_uv;
	uint8_t *vp=(uint8_t*)&rawval;

	uint8_t tx[] = {
		0xFF, 0xFF, 0xFF
	};
		// 0x80, 0x00, 0xFF, 0xFF,

	uint8_t rx[ARRAY_SIZE(tx)] = {0, };

	struct spi_ioc_transfer xfer = {
		// conversion-mode read
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.cs_change = 0, /* keep CS low (active) */
		.len = ARRAY_SIZE(rx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = 8,
	};

	printf("xfer.txbuf : %lu \n\n", xfer.tx_buf); 
	printf("xfer.rxbuf : %lu \n\n",xfer.rx_buf); 
	printf("xfer.cs_change : %d \n\n", xfer.cs_change); 
	printf("xfer.len: %d \n\n", xfer.len); 
	printf("val: %d mV\n", val_mv);
	printf("xfer.delay_usecs : %d \n\n", xfer.delay_usecs); 
	printf("xfer.speed_hz : %d \n\n", xfer.speed_hz); 
	/* DAC p.19
		V_out = V_ref * (D/2**16)
		v_out / v_ref = D / 2**16
		(v_out / v_ref) * 2**16  = D
		*/

		D_val16 = (val_mv * 65536) / v_ref;

		printf("D: %d\n",  D_val16);

		tx[2] = (D_val16 & 0x000F) << 4;

		tx[1] = (D_val16 & 0x0FF0) >> 4;

		tx[0] = 0x30 | (D_val16 & 0xF000) >> 12;


		

		printf("%d\n", tx[0]); 
		printf("%d\n", tx[1]); 
		printf("%d\n", tx[2]); 
		printf("%u\n", tx[0]); 
		
		printf("fd is %d",fd);
		
		if (ioctl(fd, SPI_IOC_MESSAGE(1), &xfer) < 0)
			pabort("can't send spi message");

}


static void print_usage(const char *prog)
{
	printf("Usage: %s [-DsbdlHOLC3]\n", prog);
	puts("  -D --device   device to use (default /dev/spidev1.1)\n"
	     "  -s --speed    max speed (Hz)\n"
	     "  -d --delay    delay (usec)\n"
	     "  -b --bpw      bits per word \n"
	     "  -v --value    V_out (mV)\n"
	     "  -l --loop     loopback\n"
	     "  -H --cpha     clock phase\n"
	     "  -O --cpol     clock polarity\n"
	     "  -L --lsb      least significant bit first\n"
	     "  -C --cs-high  chip select active high\n"
	     "  -3 --3wire    SI/SO signals shared\n");
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "bpw",     1, 0, 'b' },
			{ "value",   1, 0, 'v' },
			{ "loop",    0, 0, 'l' },
			{ "cpha",    0, 0, 'H' },
			{ "cpol",    0, 0, 'O' },
			{ "lsb",     0, 0, 'L' },
			{ "cs-high", 0, 0, 'C' },
			{ "3wire",   0, 0, '3' },
			{ "no-cs",   0, 0, 'N' },
			{ "ready",   0, 0, 'R' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "D:s:d:b:v:lHOLC3NR", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 's':
			speed = atoi(optarg);
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'b':
			bits = atoi(optarg);
			break;
		case 'v':
			val_mv = atoi(optarg);
			break;
		case 'l':
			mode |= SPI_LOOP;
			break;
		case 'H':
			mode |= SPI_CPHA;
			break;
		case 'O':
			mode |= SPI_CPOL;
			break;
		case 'L':
			mode |= SPI_LSB_FIRST;
			break;
		case 'C':
			mode |= SPI_CS_HIGH;
			break;
		case '3':
			mode |= SPI_3WIRE;
			break;
		case 'N':
			mode |= SPI_NO_CS;
			break;
		case 'R':
			mode |= SPI_READY;
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;

	parse_opts(argc, argv);
	
	printf("O_RDWR %d",O_RDWR);
	
	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	printf("device is %c",device);	
	
	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	resolution = (v_ref * 1000)/ 65536 ;  // micro-volts


	printf("bits: %d\n", dac_bits);
	printf("v_ref: %d mV\n", v_ref);
	printf("res: %d uV\n", resolution);

	printf("device: %s\n", device);
	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("delay: %d\n", delay);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	stream(fd);

	close(fd);

	return ret;
}
