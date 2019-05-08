#include "kernel.h"
#include "utils.h"
#include "char.h"

uint32 vga_index;
static uint32 next_line_index = 1;
uint8 g_fore_color = WHITE, g_back_color = BLUE;
int digit_ascii_codes[10] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};

/*
this is same as we did in our assembly code for vga_print_char

vga_print_char:
  mov di, word[VGA_INDEX]
  mov al, byte[VGA_CHAR]

  mov ah, byte[VGA_BACK_COLOR]
  sal ah, 4
  or ah, byte[VGA_FORE_COLOR]

  mov [es:di], ax

  ret

*/
uint16 vga_entry(unsigned char ch, uint8 fore_color, uint8 back_color) 
{
  uint16 ax = 0;
  uint8 ah = 0, al = 0;

  ah = back_color;
  ah <<= 4;
  ah |= fore_color;
  ax = ah;
  ax <<= 8;
  al = ch;
  ax |= al;

  return ax;
}

void clear_vga_buffer(uint16 **buffer, uint8 fore_color, uint8 back_color)
{
  uint32 i;
  for(i = 0; i < BUFSIZE; i++){
    (*buffer)[i] = vga_entry(NULL, fore_color, back_color);
  }
  next_line_index = 1;
  vga_index = 0;
}

void init_vga(uint8 fore_color, uint8 back_color)
{
  vga_buffer = (uint16*)VGA_ADDRESS;
  clear_vga_buffer(&vga_buffer, fore_color, back_color);
  g_fore_color = fore_color;
  g_back_color = back_color;
}

void print_new_line()
{
  if(next_line_index >= 55){
    next_line_index = 0;
    clear_vga_buffer(&vga_buffer, g_fore_color, g_back_color);
  }
  vga_index = 80*next_line_index;
  next_line_index++;
}

void print_char(char ch, int f, int b)
{
  vga_buffer[vga_index] = vga_entry(ch, f, b);
  vga_index++;
}

void print_string(char *str, int f, int b)
{
  uint32 index = 0;
  while(str[index]){
    print_char(str[index],f,b);
    index++;
  }
}

void print_int(int num, int f, int b)
{
  char str_num[digit_count(num)+1];
  itoa(num, str_num);
  print_string(str_num, f, b);
}

uint8 inb(uint16 port)
{
  uint8 ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "d"(port));
  return ret;
}

void outb(uint16 port, uint8 data)
{
  asm volatile("outb %0, %1" : "=a"(data) : "d"(port));
}

char get_input_keycode()
{
  char ch = 0;
  while((ch = inb(KEYBOARD_PORT)) != 0){
    if(ch > 0)
      return ch;
  }
  return ch;
}

/*
keep the cpu busy for doing nothing(nop)
so that io port will not be processed by cpu
here timer can also be used, but lets do this in looping counter
*/
void wait_for_io(uint32 timer_count)
{
  while(1){
    asm volatile("nop");
    timer_count--;
    if(timer_count <= 0)
      break;
    }
}

void sleep(uint32 timer_count)
{
  wait_for_io(timer_count);
}



void input()
{
  char ch = 0;
  char keycode = 0;
  char command[100];
  int j = 0;
  do{
    keycode = get_input_keycode();
    if(keycode == KEY_ENTER){
      int i;
      print_new_line();

      if(str_cmp(command,"CONFIG"))
      {
          config_menu();
      }

      for(i=0; i<100; i++)
      {
          command[i] = NULL;
      }


      print_new_line();
      print_string(">",0,WHITE);

      j=0;
    }else{
      ch = get_ascii_char(keycode);
      print_char(ch, BLUE, WHITE);
      command[j] = ch;
      j++;
    }





    sleep(0x0A000000);
  }while(1);
}


int str_cmp(char* a, char* b )
{
  int i;
  for(i = 0; i<sizeof(a); i++)
  {
    if(a[i] != b[i])
    {
      return 0;
    }
  }
  return 1;
}

void kernel_entry()
{
  init_vga(BLUE, WHITE);
  print_string("Tupux Kernel v1.0", 0xf5, WHITE);
  print_new_line();
  input();

}

void config_menu()
{
  char keycode;
  char ch;
  int dec = 0;
  int selected = 0;
  int exit = 0;
  do
  {
    init_vga(WHITE, 0x05);
    print_string("                 Config Menu                 ",WHITE,1);
    print_new_line();


    print_string("1 - About",WHITE,0x05);
    print_new_line();
    print_string("2 - Change Colors",WHITE,0x05);
    print_new_line();
    print_string("3 - Exit",WHITE,0x05);
    
    dec = get_input_keycode();
      switch(dec)
      {
        case 0x02:
          init_vga(WHITE, 0x05);
          print_string("                 About                 ",WHITE,1);
          print_new_line();
          print_string("Tupux kernel: Coded by Manuel Candil Baeza",WHITE,0x03);
          keycode = get_input_keycode();
          break;
        case 0x04:
          exit = 1;
          break;
      }
    if(exit)
    {
      init_vga(BLUE, WHITE);
      break;
    }
    sleep(0x0A000000);
  } while (1);
  
}