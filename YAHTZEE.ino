#include <LiquidCrystal.h>
#include "Keypad.h"
 //lcd board
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
//keypad keybinds
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}
};
int roundCount[4] = {0, 0, 0, 0};
//Useable catagories
const int totalCategories = 13;
const String categoryNames[totalCategories] = {
  "Ones", "Twos", "Threes", "Fours", "Fives", "Sixes",
  "3 of Kind", "4 of Kind", "Full House", "Small St.", "Large St.", "Yahtzee", "Chance"
};
//keypad setup
byte rowPins[ROWS] = {21, 20, 19, 18};
byte colPins[COLS] = {17, 16, 15, 14};    
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
 //player class for each player, own unique scores
struct Player {
  String name;
  int ones = 0, twos = 0, threes = 0, fours = 0, fives = 0, sixes = 0;
  int threeOfAKind = 0, fourOfAKind = 0, fullHouse = 0;
  int smallStraight = 0, largeStraight = 0, yahtzee = 0, chance = 0;
  int dice[5] = {0, 0, 0, 0, 0};
 
  //For seeing if the catagory is already filled when scoring
  bool categoryFilled[13] = {false};  
 
  //Idk if this is even used
  Player(String n) {
    name = n;
  }
};
//Creates 4 Players just incase.
Player players[4] = {
  Player("Player 1"),
  Player("Player 2"),
  Player("Player 3"),
  Player("Player 4")
};
//Player select how many players
int activePlayers = 0;        
//Beginning bool to determine if the player is still determining how many players. If its done it turns to false  
bool selectingPlayers = true;    
//bool used in if statements to note if the player is rolling or displaying categories
bool displayedCategories = false;
int currentPlayer = 0;
//What roll
int rollCount = 1;
//Holder to see which dice is kept and released
bool keep[5] = {false, false, false, false, false};
//bool to determine if the player wants to input scores or keep rolling.
bool awaitingCategoryInput = false;


void setup() {
  //Player select
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("Select Players:");
  lcd.setCursor(0, 1);
  lcd.print("1-4");


  randomSeed(analogRead(A0));
}


void loop() {
  char key = keypad.getKey();
  //CHoose how many players
  if (selectingPlayers) {
    if (key >= '1' && key <= '4') {
      activePlayers = key - '0';
      selectingPlayers = false;
      lcd.clear();
      lcd.print("Players: " + String(activePlayers));
      delay(1000);
      StartTurn();
    }
    return;
  }
  //show to screen which key is pressed
  if (key != NO_KEY) {
  Serial.print("Key Pressed: ");
  Serial.println(key);
  //Begin catagory input
  if (awaitingCategoryInput) {
    HandleCategoryInput(key);
    return;
  }


 


  //Keep which dice
  if (key >= '1' && key <= '5') {
  int dieIndex = key - '1';
  keep[dieIndex] = !keep[dieIndex];
  Serial.print("Die ");
  Serial.print(dieIndex + 1);
  Serial.println(keep[dieIndex] ? " KEPT" : " RELEASED");
  DisplayDice();
  return;
}


  // Reroll
  if (key == '*') {
    if (rollCount < 3) {
      rollCount++;
      RerollDice();
    } else {
      Serial.println("Max rolls reached. Use # to score.");
    }
    return;
  }


  // Ready to pick a scoring category
  if (key == '#') {
    Serial.println("Choose category to score:");
    Serial.println("1-6 = Upper, 7 = 3OK, 8 = 4OK, 9 = FH, A = SS, B = LS, C = Y, D = Chance, # = Cancel");
    awaitingCategoryInput = true;
    return;
  }
}
}


void StartTurn() {
  rollCount = 1;
  displayedCategories = false;
  for (int i = 0; i < 5; i++) {
    keep[i] = false;
  }
  awaitingCategoryInput = false;


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(players[currentPlayer].name);


  Serial.println("---------------------------");
  Serial.println(players[currentPlayer].name + "'s Turn!");


  ShowAvailableCategories();
  RerollDice(); // Continue turn
}


void ShowAvailableCategories() {
  if (displayedCategories) return;
  displayedCategories = true;


  Player &p = players[currentPlayer];


  Serial.println("Available Categories for " + p.name + ":");
  for (int i = 0; i < totalCategories; i++) {
    if (!p.categoryFilled[i]) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Available:");
      lcd.setCursor(0, 1);
      lcd.print(categoryNames[i]);
      Serial.println(" - " + categoryNames[i]);  
      delay(500);
    }
  }


  // Show current score too
  int total = p.ones + p.twos + p.threes + p.fours + p.fives + p.sixes +
              p.threeOfAKind + p.fourOfAKind + p.fullHouse +
              p.smallStraight + p.largeStraight + p.yahtzee + p.chance;


  Serial.println("Current Score: " + String(total));
}
void DisplayDice() {
  Player &p = players[currentPlayer];


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Roll " + String(rollCount));


  lcd.setCursor(0, 1);
  for (int i = 0; i < 5; i++) {
    lcd.print(String(p.dice[i]));
    if (keep[i]) lcd.print("*");
    else lcd.print(" ");
  }
}
void RerollDice() {
  Player &p = players[currentPlayer];
  for (int i = 0; i < 5; i++) {
    if (!keep[i]) {
      p.dice[i] = random(1, 7);  // Random number from 1 to 6
    }
  }


  // Display the current dice values
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Roll " + String(rollCount));


  lcd.setCursor(0, 1);
  for (int i = 0; i < 5; i++) {
    lcd.print(String(p.dice[i]));
    if (keep[i]) lcd.print("*"); // Mark kept dice
    else lcd.print(" ");
  }
}


void HandleCategoryInput(char key) {
  Player &p = players[currentPlayer];
  int category = -1;


  switch (key) {
    case '1': category = 0; break; // Ones
    case '2': category = 1; break; // Twos
    case '3': category = 2; break; // Threes
    case '4': category = 3; break; // Fours
    case '5': category = 4; break; // Fives
    case '6': category = 5; break; // Sixes
    case '7': category = 6; break; // 3 of a Kind
    case '8': category = 7; break; // 4 of a Kind
    case '9': category = 8; break; // Full House
    case 'A': category = 9; break; // Small Straight
    case 'B': category = 10; break; // Large Straight
    case 'C': category = 11; break; // Yahtzee
    case 'D': category = 12; break; // Chance
    case '#': awaitingCategoryInput = false;
    Serial.println("Returning to Rolls");
    return; //Go back to rolls
    default:
      Serial.println("Invalid key for category.");
      return;
  }


  if (p.categoryFilled[category]) {
    Serial.println("Category already used.");
    return;
  }


  int counts[7] = {0};  // Dice value counts from 1-6
  for (int i = 0; i < 5; i++) counts[p.dice[i]]++;


  int score = 0;
  switch (category) {
    case 0 ... 5: // Ones to Sixes
      for (int i = 0; i < 5; i++) {
        if (p.dice[i] == category + 1) score += p.dice[i];
      }
      break;
    case 6: // Three of a Kind
      for (int i = 1; i <= 6; i++) if (counts[i] >= 3) score = p.dice[0] + p.dice[1] + p.dice[2] + p.dice[3] + p.dice[4];
      break;
    case 7: // Four of a Kind
      for (int i = 1; i <= 6; i++) if (counts[i] >= 4) score = p.dice[0] + p.dice[1] + p.dice[2] + p.dice[3] + p.dice[4];
      break;
    case 8: // Full House
   
    bool hasThree = false;
    bool hasTwo = false;


    for (int i = 1; i <= 6; i++) {
      if (counts[i] == 3) {
        hasThree = true;
      } else if (counts[i] == 2) {
        hasTwo = true;
      }
    }


    if (hasThree && hasTwo) {
      score = 25;
    }
    break;


   
    case 9: // Small Straight
      if ((counts[1] && counts[2] && counts[3] && counts[4]) ||
          (counts[2] && counts[3] && counts[4] && counts[5]) ||
          (counts[3] && counts[4] && counts[5] && counts[6])) {
        score = 30;
      }
      break;
    case 10: // Large Straight
      if ((counts[1] && counts[2] && counts[3] && counts[4] && counts[5]) ||
          (counts[2] && counts[3] && counts[4] && counts[5] && counts[6])) {
        score = 40;
      }
      break;
    case 11: // Yahtzee
      for (int i = 1; i <= 6; i++) if (counts[i] == 5) score = 50;
      break;
    case 12: // Chance
      for (int i = 0; i < 5; i++) score += p.dice[i];
      break;
  }


 
  switch (category) {
    case 0: p.ones = score; break;
    case 1: p.twos = score; break;
    case 2: p.threes = score; break;
    case 3: p.fours = score; break;
    case 4: p.fives = score; break;
    case 5: p.sixes = score; break;
    case 6: p.threeOfAKind = score; break;
    case 7: p.fourOfAKind = score; break;
    case 8: p.fullHouse = score; break;
    case 9: p.smallStraight = score; break;
    case 10: p.largeStraight = score; break;
    case 11: p.yahtzee = score; break;
    case 12: p.chance = score; break;
  }


  p.categoryFilled[category] = true;
  awaitingCategoryInput = false;


  Serial.println("Scored " + String(score) + " in " + categoryNames[category]);
  NextPlayer();
}


void NextPlayer() {
  roundCount[currentPlayer]++;
 
  bool gameOver = true;
  for (int i = 0; i < activePlayers; i++) {
    if (roundCount[i] < totalCategories) {
      gameOver = false;
      break;
    }
  }


  if (gameOver) {
    ShowWinner();
    return;
  }


  do {
    currentPlayer = (currentPlayer + 1) % activePlayers;
  } while (roundCount[currentPlayer] >= totalCategories);


  StartTurn();
}
void ShowWinner() {
  int highestScore = 0;
  int winnerIndex = 0;


  for (int i = 0; i < activePlayers; i++) {
    int total = players[i].ones + players[i].twos + players[i].threes + players[i].fours +
                players[i].fives + players[i].sixes + players[i].threeOfAKind + players[i].fourOfAKind +
                players[i].fullHouse + players[i].smallStraight + players[i].largeStraight +
                players[i].yahtzee + players[i].chance;


    if (total > highestScore) {
      highestScore = total;
      winnerIndex = i;
    }
  }


  Player &winner = players[winnerIndex];


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Winner:");
  lcd.setCursor(0, 1);
  lcd.print(winner.name);
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Score:");
  lcd.setCursor(0, 1);
  lcd.print(highestScore);


  // Fancy Card Display
  Serial.println("╔══════════════════════════════╗");
  Serial.println("║         🏆 YAHTZEE WINNER 🏆        ║");
  Serial.println("╠══════════════════════════════╣");
  Serial.print("║ Name: "); Serial.print(winner.name);
  int spaceCount = 26 - winner.name.length();
  for (int i = 0; i < spaceCount; i++) Serial.print(" ");
  Serial.println("║");
  Serial.print("║ Score: "); Serial.print(highestScore);
  for (int i = 0; i < 26 - String(highestScore).length() - 7; i++) Serial.print(" ");
  Serial.println("║");
  Serial.println("╠══════════════════════════════╣");


  String catNames[13] = {
    "Ones", "Twos", "Threes", "Fours", "Fives", "Sixes",
    "3 of a Kind", "4 of a Kind", "Full House", "Small Straight",
    "Large Straight", "Yahtzee", "Chance"
  };


  int catScores[13] = {
    winner.ones, winner.twos, winner.threes, winner.fours, winner.fives, winner.sixes,
    winner.threeOfAKind, winner.fourOfAKind, winner.fullHouse,
    winner.smallStraight, winner.largeStraight, winner.yahtzee, winner.chance
  };


  for (int i = 0; i < 13; i++) {
    String line = "║ " + catNames[i] + ": ";
    while (line.length() < 29 - String(catScores[i]).length()) line += " ";
    line += String(catScores[i]) + " ║";
    Serial.println(line);
  }


  Serial.println("╚══════════════════════════════╝");
  Serial.println("");
  Serial.println("Click the reset button to start a new game");
  while (true);
}

