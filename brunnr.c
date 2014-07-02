#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <sqlite3.h>

void trim(char *str);
static int callback(void *NotUsed, int argc, char **argv, char **azColName);
void setup_serial();
void setup_db();
void write_db(char *sql);
void parse_serial(char string[]);
void read_eternal();

char *portname = "/dev/ttyACM0";
char buf[256];
int fd, n;

void trim(char *str) 
{
  char *src, *dst;
  for (src = dst = str; *src != '\0'; src++) {
    *dst = *src;
    if (*dst != '\n') dst++;
  }
  *dst = '\0';
}

void setup_serial()
{
  fd = open(portname, O_RDWR | O_NOCTTY);

  struct termios toptions;

  tcgetattr(fd, &toptions);

  cfsetispeed(&toptions, B9600);
  cfsetospeed(&toptions, B9600);
  toptions.c_cflag &= ~PARENB;
  toptions.c_cflag &= ~CSTOPB;
  toptions.c_cflag &= ~CSIZE;
  toptions.c_cflag |= CS8;
  toptions.c_iflag |= ICANON;

  tcsetattr(fd, TCSANOW, &toptions);
  usleep(1000*1000);
  tcflush(fd, TCIFLUSH);
}

void read_eternal()
{
  while (1<2) {
    n = read(fd, buf, 256);
    buf[n] = 0;
    trim(buf);
    if(strlen(buf) > 0) {
      printf("%s\n", buf);
      parse_serial(buf);
    }
  }
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName) 
{
  int i;
  for(i=0;i<argc;i++) {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

void write_db(char *sql)
{
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc = 0;

  rc = sqlite3_open("brunnr.db", &db);

  if ( rc ) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
  }

  rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  sqlite3_close(db);
}

void setup_db()
{
  char *sql = "CREATE TABLE IF NOT EXISTS messages(id INTEGER PRIMARY KEY, source TEXT NOT NULL, target TEXT NOT NULL, message TEXT NOT NULL)";
  write_db(sql);
}

void parse_serial(char string[]) 
{
  char *search = "|";
  char sql[1024];
  
  char *source = strtok(string, search);
  char *target = strtok(NULL, search);
  char *message = strtok(NULL, search);

  snprintf(sql, sizeof(sql), "INSERT INTO messages(id, source, target, message) values(NULL, '%s', '%s', '%s')", source, target, message);
  write_db(sql);
}

static const struct option longopts[] =
{
  { "help", no_argument, NULL, 'h' },
  { "version", no_argument, NULL, 'v' },
  { NULL, 0, NULL, 0 }
};

int main(int argc, char *argv[])
{
  int optc;
  int t = 0,n = 0,lose = 0;
  program_name = argv[0];

  while ((optc = getopt_long (argc, argv, "g:hntv", longopts, NULL)) != -1)
    switch (optc)
    {
      case 'v':
        print_version();
        exit(EXIT_SUCCESS);
        break;
      case 'h':
        print_help();
        exit(EXIT_SUCCESS);
        break;
    }
  if (lose || optind < argc)
  {
    if (optind < argc) 
      fprintf(stderr, _("%s: extra operand: %\n"),
          program_name, argv[optind]);
    fprintf(stderr, _"Try '%s --help' for more information.\n"), program_name);
    exit(EXIT_FAILURE);
  }
  setup_db();
  char rawSerial[] = "uno3|rss3|get:unread|";
  parse_serial(rawSerial);
}

static void print_help()
{
  printf(_("Usage; %s [OPTION]. . .\n") project_name);
