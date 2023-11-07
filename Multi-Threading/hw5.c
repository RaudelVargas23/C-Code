#include"elevator.h"
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>

//same struct as main with some changes
struct Elevator {
  pthread_mutex_t elevatorLock;//elevatorLock
  pthread_barrier_t elevatorBarrier;
  pthread_barrier_t  passBarrier;
  int currentFloor;
  int direction;
  int toFloor;
  int occupancy;
  //ENUM DO SOME RESEARCH ON THIS
  enum { ELEVATOR_ARRIVED,ELEVATOR_OPEN,ELEVATOR_CLOSED
  }currentState;
 
}elevators[ELEVATORS];

void scheduler_init() {
  for (int i = 0; i < ELEVATORS; i++) {
    elevators[i].currentFloor = 0;
    elevators[i].direction = -1;
    elevators[i].toFloor = 0;//assuming all elevator start at ground level  maybe an error idk
    elevators[i].occupancy = 0;
    elevators[i].currentState = ELEVATOR_ARRIVED;
    pthread_mutex_init(&elevators[i].elevatorLock, 0);//from lab
    pthread_barrier_init(&elevators[i].elevatorBarrier, 0, 2);//from lab
    pthread_barrier_init(&elevators[i].passBarrier, 0, 2);//from lab
  }
}
//figure out what the fuck void(* enter) means
//ask TA later
void passenger_request(int passenger, int from_floor, int to_floor,void( * enter)(int, int),void( * exit)(int, int)) {
  //pick a random elevator 
  //MIGHT NOT WORK CHECK BACK HERE IF ISSUES
  int randomEle = random() % ELEVATORS;
  while (elevators[randomEle].occupancy == 1) {
    randomEle = random() % ELEVATORS;
  }
  
  pthread_mutex_lock( & elevators[randomEle].elevatorLock);
  elevators[randomEle].toFloor = from_floor;
  pthread_barrier_wait( & elevators[randomEle].elevatorBarrier);
  
  if (elevators[randomEle].occupancy == 0) {
    enter(passenger, randomEle);
    elevators[randomEle].occupancy++;
  }

  elevators[randomEle].toFloor = to_floor;
  pthread_barrier_wait( & elevators[randomEle].passBarrier);
  pthread_barrier_wait( & elevators[randomEle].elevatorBarrier);
  
  if (elevators[randomEle].occupancy == 1) {
    exit(passenger, randomEle);
    elevators[randomEle].occupancy--;
  }
  elevators[randomEle].toFloor = -1;
  pthread_barrier_wait( & elevators[randomEle].passBarrier);
  pthread_mutex_unlock( & elevators[randomEle].elevatorLock);

}

void elevator_ready(int elevator, int at_floor,void( * move_direction)(int, int),void( * door_open)(int), void( * door_close)(int)) {
  
  if (elevators[elevator].toFloor == at_floor && elevators[elevator].currentState == ELEVATOR_ARRIVED) {
    door_open(elevator);
    elevators[elevator].currentState = ELEVATOR_OPEN;
    pthread_barrier_wait( & elevators[elevator].elevatorBarrier);
    pthread_barrier_wait( & elevators[elevator].passBarrier);
  } 
  else if (elevators[elevator].currentState == ELEVATOR_OPEN) {
    door_close(elevator);
    elevators[elevator].currentState = ELEVATOR_CLOSED;
  } 
  else {
  if (at_floor == 0 || at_floor == FLOORS - 1){
    elevators[elevator].direction *= -1;
  } 
  if (elevators[elevator].toFloor != -1) {
    move_direction(elevator, elevators[elevator].direction);
    elevators[elevator].currentFloor = at_floor + elevators[elevator].direction;
  }
  elevators[elevator].currentState = ELEVATOR_ARRIVED;

  }
}
