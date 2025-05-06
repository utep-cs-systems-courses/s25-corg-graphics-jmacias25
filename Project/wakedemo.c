#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"

// WARNING: LCD DISPLAY USES P1.0.  Do not touch!!! 

#define LED BIT6		/* note that bit zero req'd for display */

#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8

#define SWITCHES 15

#define HIT_ROW (screenHeight -50)
#define BLOCK_SIZE 10

char blue = 31, green = 0, red = 31;
unsigned char step = 0;
int score = 0;

static char 
switch_update_interrupt_sense()
{
  char p2val = P2IN;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (p2val & SWITCHES);	/* if switch up, sense down */
  P2IES &= (p2val | ~SWITCHES);	/* if switch down, sense up */
  return p2val;
}

void 
switch_init()			/* setup switch */
{  
  P2REN |= SWITCHES;		/* enables resistors for switches */
  P2IE |= SWITCHES;		/* enable interrupts from switches */
  P2OUT |= SWITCHES;		/* pull-ups for switches */
  P2DIR &= ~SWITCHES;		/* set switches' bits for input */
  switch_update_interrupt_sense();
}

int switches = 0;

void
switch_interrupt_handler()
{
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;
}


// axis zero for col, axis 1 for row

short drawPos[2] = {1,10}, controlPos[4][2] = {{20, 0},{45,0},{70,0},{95,0}};
short colVelocity = 1, colLimits[2] = {1, screenWidth};
short rowVelocity = 2, rowLimits[2] = {1, screenHeight};


void
draw_note(int col, int row, unsigned short color)
{
  fillRectangle(col-1, row-1, 10, 10, color);
}

void
draw_note2(int col, int row, unsigned short color)
{
  fillRectangle(col+10, row-1, 10, 10, color);
}

void
draw_note3(int col, int row, unsigned short color)
{
  fillRectangle(col+15, row-1, 10, 10, color);
}

void
draw_note4(int col, int row, unsigned short color)
{
  fillRectangle(col+25, row-1, 10, 10, color);
}

short prevY[4] = {0, 0, 0, 0};

void
screen_update_ball()
{
  for (int i = 0; i < 4; i++){ 
  
    fillRectangle(controlPos[i][0], prevY[i], BLOCK_SIZE, BLOCK_SIZE, COLOR_BLUE);
  
    prevY[i] = controlPos[i][1];
    
   if(i == 0){
     draw_note(controlPos[i][0], controlPos[i][1], COLOR_GREEN);}
   else if(i ==1){
       draw_note2(controlPos[i][0], controlPos[i][1], COLOR_RED);}
   else if(i == 2){
     draw_note3(controlPos[i][0], controlPos[i][1], COLOR_YELLOW);}
   else {
     draw_note4(controlPos[i][0], controlPos[i][1], COLOR_PINK);
   }
  }
}

void
draw_score(){
  char scoreStr[10];
  sprintf(scoreStr, "Score: %d", score);
  drawString5x7(screenWidth - 60, 5, scoreStr, COLOR_WHITE, COLOR_RED);
}
  

short redrawScreen = 1;
u_int controlFontColor = COLOR_GREEN;

void wdt_c_handler()
{
  static int secCount = 0;
  secCount++;

  if (secCount >= 25){
    secCount = 0;

    for (int i = 0; i < 4; i++){

      controlPos[i][1] += 2;

      if ((switches & (1 << i)) &&
	  (controlPos[i][1] + BLOCK_SIZE >= HIT_ROW) &&
	  (controlPos[i][1] <= HIT_ROW + BLOCK_SIZE)) {
	score++;
	controlPos[i][1] = 0;
      }

      if (controlPos[i][1] > screenHeight){
	controlPos[i][1] = 0;
      }
    }

    redrawScreen = 1;
  }
}
  
void update_shape();

void main()
{
  
  P1DIR |= LED;		/**< Green led on when CPU on */
  P1OUT |= LED;
  configureClocks();
  lcd_init();
  switch_init();
  
  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */
  
  clearScreen(COLOR_BLUE);
  for(int i = 0; i <screenWidth; i += 5){
    fillRectangle(i, screenHeight - 50, 3, 3, COLOR_WHITE);
  } 
  while (1) {			/* forever */
    if (redrawScreen) {
      redrawScreen = 0;
      update_shape();
    }
    P1OUT &= ~LED;	/* led off */
    or_sr(0x10);	/**< CPU OFF */
    P1OUT |= LED;	/* led on */
  }
}



    
void
update_shape()
{
  screen_update_ball();
  draw_score();
 
}
   


void
__interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES) {	      /* did a button cause this interrupt? */
    P2IFG &= ~SWITCHES;		      /* clear pending sw interrupts */
    switch_interrupt_handler();	/* single handler for all switches */
  }
}
