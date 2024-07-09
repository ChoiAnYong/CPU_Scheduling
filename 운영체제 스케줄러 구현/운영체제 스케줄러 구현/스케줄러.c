#include<stdio.h>
#include<stdlib.h>
#include<Windows.h>
#pragma warning(disable:4996)

const int RT = 1;//응답시간

//프로세스의 정보를 담을 구조체
typedef struct process {
	int pro_id, arr_t, ser_t, pro_pri, res_t, wait_t, turn_a_t, index, count;//프로세스 ID, 도착 시간, 서비스 시간, 우선 순위, 응답 시간, 대기 시간, 반환 시간, 저장된 배열의 인덱스, 현재 수행시간
	double HRN_pro_pri;//HRN에서 구한 우선순위는 정수형이 아닌 실수형이 나오므로 실수형 우선순위를 저장하기 위한 변수
}process;

//스케줄러의 정보를 담을 구조체
typedef struct scheduler {
	int pro_count, time_slice;//프로세스 개수, 타임슬라이스
	double awt, art, att;//평균 대기시간, 평균 응답시간, 평균 반환시간
	process* list;//프로세스를 저장할 배열
}scheduler;

//원형큐로 준비큐를 구현하기 위한 구조체
typedef struct {
	int size;//큐의 크기
	int rear;//큐의 마지막 인덱스
	int front;//큐의 첫 번째 인덱스(원형큐이므로 큐의 첫 번째 원소 인덱스는 아님) 
	int count;//현재 큐에 있는 프로세스의 개수
	process* list;//큐에 들어온 프로세스의 정보를 저장할 배열
}queuetype;

//큐 초기화 함수
void init_queue(queuetype* q, scheduler* S) {
	q->front = 0;//front를 0으로 초기화
	q->rear = 0;//rear를 0으로 초기화
	q->count = 0;//현재 큐에 있는 프로세스의 개수를 0으로 초기화 
	q->size = S->pro_count + 1;//원형큐이므로 큐의 크기를 스케줄러에 있는 프로세스의 개수 +1로 초기화
	q->list = (process*)malloc(sizeof(process) * q->size);//프로세스의 정보를 저장할 배열을 큐의 크기만큼 동적 할당
}

//큐 공백 검사 함수
int is_empty(queuetype* q) {
	return (q->rear == q->front);
}

//큐 포화 검사 함수
int is_full(queuetype* q) {
	return ((q->rear + 1) % q->size == q->front);
}

//큐 인큐 함수
void enqueue(queuetype* q, process p) {
	if (is_full(q)) {
		fprintf(stderr, "큐가 포화상태 입니다.\n");
		return;
	}

	q->rear = (q->rear + 1) % q->size;
	q->list[q->rear] = p;
	q->count++;//큐에 프로세스가 들어왔으므로 count 증가
}

//큐 디큐 함수
process dequeue(queuetype* q) {
	if (is_empty(q)) {
		fprintf(stderr, "큐가 공백상태입니다\n");
		return;
	}

	q->front = (q->front + 1) % q->size;
	q->count--;//큐에서 프로세스가 나갔으므로 count 감소
	return q->list[q->front];
}

//준비큐에 들어온 프로세스를 서비스 시간 기준으로 정렬하는 함수
void queue_ser_t_sort(queuetype* q) {
	process temp;//정렬하기 위해 잠시 프로세스의 정보를 저장할 변수

	//버블정렬
	for (int i = q->count; i > 0; i--) {
		for (int j = 0; j < i - 1; j++) {
			//현재 프로세스의 서비스 시간이 다음 프로세스의 서비스 시간보다 긴 경우 정렬
			if (q->list[(q->front + 1 + j) % q->size].ser_t > q->list[(q->front + 2 + j) % q->size].ser_t) {
				temp = q->list[(q->front + 2 + j) % q->size];
				q->list[(q->front + 2 + j) % q->size] = q->list[(q->front + 1 + j) % q->size];
				q->list[(q->front + 1 + j) % q->size] = temp;
			}
			//현재 프로세스의 서비스 시간과 다음 프로세스의 서비스 시간이 같을 경우 id순으로 정렬
			else if (q->list[(q->front + 1 + j) % q->size].ser_t == q->list[(q->front + 2 + j) % q->size].ser_t && q->list[(q->front + 1 + j) % q->size].pro_id > q->list[(q->front + 2 + j) % q->size].pro_id) {
				temp = q->list[(q->front + 2 + j) % q->size];
				q->list[(q->front + 2 + j) % q->size] = q->list[(q->front + 1 + j) % q->size];
				q->list[(q->front + 1 + j) % q->size] = temp;
			}
		}
	}
}

//준비큐에 들어온 프로세스를 우선순위 기준으로 정렬하는 함수
void queue_pro_pri_sort(queuetype* q) {
	process temp;//정렬하기 위해 잠시 프로세스의 정보를 저장할 변수

	//버블정렬
	for (int i = q->count; i > 0; i--) {
		for (int j = 0; j < i - 1; j++) {
			//현재 프로세스의 우선순위가 다음 프로세스보다 낮을 정렬(숫자가 작을수록 우선순위가 높음)
			if (q->list[(q->front + 1 + j) % q->size].pro_pri > q->list[(q->front + 2 + j) % q->size].pro_pri) {
				temp = q->list[(q->front + 2 + j) % q->size];
				q->list[(q->front + 2 + j) % q->size] = q->list[(q->front + 1 + j) % q->size];
				q->list[(q->front + 1 + j) % q->size] = temp;
			}
			//현재 프로세스의 우선순위와 다음 프로세스의 우선순위가 같을 경우 id순으로 정렬
			else if (q->list[(q->front + 1 + j) % q->size].pro_pri == q->list[(q->front + 2 + j) % q->size].pro_pri && q->list[(q->front + 1 + j) % q->size].pro_id > q->list[(q->front + 2 + j) % q->size].pro_id) {
				temp = q->list[(q->front + 2 + j) % q->size];
				q->list[(q->front + 2 + j) % q->size] = q->list[(q->front + 1 + j) % q->size];
				q->list[(q->front + 1 + j) % q->size] = temp;
			}
		}
	}
}

//스케줄러에 들어온 프로세스를 도착순으로 정렬하는 함수
void arr_t_sort(scheduler* S) {
	process temp;//정렬하기 위해 잠시 프로세스의 정보를 저장할 변수
	int index_temp;//정렬할 때 저장된 배열의 인덱스도 바꾸기 위해 인덱스 정보를 저장할 변수
	//버블정렬
	for (int i = S->pro_count; i > 0; i--) {
		for (int j = 0; j < i - 1; j++) {
			//현재 프로세스의 도착시간이 다음 프로세스보다 늦을 경우 정렬
			if (S->list[j].arr_t > S->list[j + 1].arr_t) {
				temp = S->list[j + 1];
				S->list[j + 1] = S->list[j];
				S->list[j] = temp;
				//아래 과정을 안하면 정렬되어 변경된 인덱스가 프로세스 정보에 있는 인덱스 변수에는 변경되지 않음
				index_temp = S->list[j].index;
				S->list[j].index = S->list[j + 1].index;
				S->list[j + 1].index = index_temp;
			}
			//도착시간이 같을 경우 id순으로 정렬
			else if (S->list[j].arr_t == S->list[j + 1].arr_t && S->list[j].pro_id > S->list[j + 1].pro_id) {
				temp = S->list[j + 1];
				S->list[j + 1] = S->list[j];
				S->list[j] = temp;
				//아래 과정을 안하면 정렬되어 변경된 인덱스가 프로세스 정보에 있는 인덱스 변수에는 변경되지 않음
				index_temp = S->list[j].index;
				S->list[j].index = S->list[j + 1].index;
				S->list[j + 1].index = index_temp;
			}
		}
	}
}

//스케줄러에 들어온 프로세스를 id순으로 정렬하는 함수
void pro_id_sort(scheduler* S) {
	process temp;
	int index_temp;
	//버블정렬
	for (int i = S->pro_count; i > 0; i--) {
		for (int j = 0; j < i - 1; j++) {
			if (S->list[j].pro_id > S->list[j + 1].pro_id) {
				temp = S->list[j + 1];
				S->list[j + 1] = S->list[j];
				S->list[j] = temp;
				//아래 과정을 안하면 정렬되어 변경된 인덱스가 프로세스 정보에 있는 인덱스 변수에는 변경되지 않음
				index_temp = S->list[j].index;
				S->list[j].index = S->list[j + 1].index;
				S->list[j + 1].index = index_temp;
			}
		}
	}
}

//HRN 스케줄링에서 준비큐에 있는 프로세스들의 우선 순위를 구하고 정렬해주는 함수
void HRN_Priority(queuetype* q, int time) {
	process temp;
	//현재 시간 기준으로 큐에 있는 프로세스의 우선순위를 계산
	for (int i = 0; i < q->count; i++) {
		q->list[q->front + 1 + i].HRN_pro_pri = (double)(time - q->list[q->front + 1 + i].arr_t + q->list[q->front + 1 + i].ser_t) / q->list[q->front + 1 + i].ser_t;
	}
	//버블정렬
	for (int i = q->count; i > 0; i--) {
		for (int j = 0; j < i - 1; j++) {
			//현재 프로세스의 HRN 우선순위가 다음 프로세스의 HRN 우선순위보다 낮을 경우(숫자가 클수록 우선순위가 높음)
			if (q->list[(q->front + 1 + j) % q->size].HRN_pro_pri < q->list[(q->front + 2 + j) % q->size].HRN_pro_pri) {
				temp = q->list[(q->front + 2 + j) % q->size];
				q->list[(q->front + 2 + j) % q->size] = q->list[(q->front + 1 + j) % q->size];
				q->list[(q->front + 1 + j) % q->size] = temp;
			}
			//우선순위가 같을 경우 id순으로 정렬
			else if (q->list[(q->front + 1 + j) % q->size].HRN_pro_pri == q->list[(q->front + 2 + j) % q->size].HRN_pro_pri && q->list[(q->front + 1 + j) % q->size].pro_id > q->list[(q->front + 2 + j) % q->size].pro_id) {
				temp = q->list[(q->front + 2 + j) % q->size];
				q->list[(q->front + 2 + j) % q->size] = q->list[(q->front + 1 + j) % q->size];
				q->list[(q->front + 1 + j) % q->size] = temp;
			}
		}
	}
}

//스케줄러의 내용을 출력해주는 함수(대기시간, 응답시간, 반환 시간)
void print_scheduler(scheduler* S) {
	//id순으로 출력하기 위해 pro_id_sort함수 호출
	pro_id_sort(S);//스케줄러에 있는 프로세스를 id순으로 정렬
	printf("\n┌────┬─────────────┐\n");
	printf("│ ID │   대기시간  │\n");
	printf("├────┼─────────────┤\n");
	for (int i = 0; i < S->pro_count; i++)
		printf("│P%-2d │     %3d     │\n", S->list[i].pro_id, S->list[i].wait_t);
	printf("├────┼─────────────┤\n");
	printf("│평균│    ");
	if ((int)S->awt / 10 == 0)printf(" ");//평균이 한자리 나와 표가 깨지는 경우를 방지하기 위한 조건문
	printf("%.2f    │\n", S->awt);//소수점 2자리까지만 출력
	printf("└────┴─────────────┘");
	printf("\n┌────┬─────────────┐\n");
	printf("│ ID │   응답시간  │\n");
	printf("├────┼─────────────┤\n");
	for (int i = 0; i < S->pro_count; i++)
		printf("│P%-2d │     %3d     │\n", S->list[i].pro_id, S->list[i].res_t);
	printf("├────┼─────────────┤\n");
	printf("│평균│    ");
	if ((int)S->art / 10 == 0)printf(" ");//평균이 한자리 나와 표가 깨지는 경우를 방지하기 위한 조건문
	printf("%.2f    │\n", S->art);//소수점 2자리까지만 출력	
	printf("└────┴─────────────┘");
	printf("\n┌────┬─────────────┐\n");
	printf("│ ID │   반환시간  │\n");
	printf("├────┼─────────────┤\n");
	for (int i = 0; i < S->pro_count; i++)
		printf("│P%-2d │     %3d     │\n", S->list[i].pro_id, S->list[i].turn_a_t);
	printf("├────┼─────────────┤\n");
	printf("│평균│    ");
	if ((int)S->awt / 10 == 0)printf(" ");//평균이 한자리 나와 표가 깨지는 경우를 방지하기 위한 조건문
	printf("%.2f    │\n", S->att);//소수점 2자리까지만 출력
	printf("└────┴─────────────┘\n\n");
}

//메뉴 출력 함수
void print_menu() {
	printf("==================MENU==================\n");
	printf("1. FCFS 스케줄링\n");
	printf("2. SJF 스케줄링\n");
	printf("3. HRN 스케줄링\n");
	printf("4. 비선점형 우선순위 스케줄링\n");
	printf("5. 라운드로빈 스케줄링\n");
	printf("6. SRT 스케줄링\n");
	printf("7. 선점형 우선순위 스케줄링\n");
	printf("8. 종료\n");
	printf("========================================\n");
}

//파일에서 읽어온 프로세스 정보를 출력해주는 함수
void print_process_file(scheduler* S) {
	pro_id_sort(S);
	printf("\t    <프로세스 정보>");
	printf("\n┌────┬──────────┬────────────┬──────────┐\n");
	printf("│ ID │ 도착시간 │ 서비스시간 │ 우선순위 │\n");
	printf("├────┼──────────┼────────────┼──────────┤\n");
	for (int i = 0; i < S->pro_count; i++)
		printf("│P%-2d │   %3d    │    %3d     │   %3d    │\n", S->list[i].pro_id, S->list[i].arr_t, S->list[i].ser_t, S->list[i].pro_pri);
	printf("└────┴──────────┴────────────┴──────────┘\n");
	printf("\t   [타임 슬라이스 %d]\n", S->time_slice);
}

//스케줄러 정보 초기화 함수(ID, 도착시간, 서비스 시간, 우선순위, 프로세스 개수, 타임슬라이스, 인덱스 제외)
void reset(scheduler* S) {
	for (int i = 0; i < S->pro_count; i++) {
		S->list[i].res_t = 0;
		S->list[i].wait_t = 0;
		S->list[i].turn_a_t = 0;
		S->list[i].count = 0;
		S->list[i].HRN_pro_pri = 0.0;
	}
	S->art = S->att = S->awt = 0.0;
}

//FCFS 스케줄링 함수
void FCFS_Scheduling(scheduler* S) {
	printf("<FCFS 스케줄링>\n[간트차트]\n");
	reset(S);//스케줄러 정보 초기화(ID, 도착시간, 서비스 시간, 우선순위, 프로세스 개수, 타임슬라이스, 인덱스 제외)
	arr_t_sort(S);//도착 순서로 정렬
	queuetype q;//큐 생성
	process temp;//큐에서 나온 프로세스의 정보를 저장할 변수
	int run_time = 0, wt_sum = 0, tt_sum = 0, rt_sum = 0;//현재 시간, 대기시간 합, 반환시간 합, 응답시간 합을 저장할 변수

	init_queue(&q, S);//큐 초기화
	for (int i = 0; i < S->pro_count; i++) {
		enqueue(&q, S->list[i]);//도착순으로 큐에 삽입
	}

	for (int i = 0; i < S->pro_count; i++) {
		temp = dequeue(&q);//하나씩 디큐
		S->list[temp.index].wait_t = run_time - temp.arr_t;//현재 프로세스의 대기시간 계산 후 변경
		S->list[temp.index].res_t = S->list[temp.index].wait_t + RT;//현재 프로세스의 반응시간 계산 후 변경
		S->list[temp.index].turn_a_t = S->list[temp.index].wait_t + temp.ser_t;//현재 프로세스의 반환시간 계산 후 변경

		for (int i = 0; i < temp.ser_t; i++)//현재 프로세스의 서비스 시간만큼 간트차트 출력
			printf("P%d", temp.pro_id);
		printf("[%d] | ", temp.ser_t);//다음 프로세스와 구분하고 서비스 시간을 나타내기 위한 출력문
		run_time += temp.ser_t;//현재시간을 현재 프로세스의 서비스 시간만큼 증가
		wt_sum += S->list[temp.index].wait_t;//프로세스들의 대기시간 합을 구하기 위해 현재 프로세스의 대기시간 더하기
		tt_sum += S->list[temp.index].turn_a_t;//프로세스들의 반환시간 합을 구하기 위해 현재 프로세스의 반환시간 더하기
		rt_sum += S->list[temp.index].res_t;//프로세스들의 응답시간 합을 구하기 위해 현재 프로세스의 응답시간 더하기
	}
	S->awt = (double)wt_sum / S->pro_count;//모든 프로세스가 끝났으므로 평균을 구해서 스케줄러에 저장
	S->att = (double)tt_sum / S->pro_count;//모든 프로세스가 끝났으므로 평균을 구해서 스케줄러에 저장
	S->art = (double)rt_sum / S->pro_count;//모든 프로세스가 끝났으므로 평균을 구해서 스케줄러에 저장
	print_scheduler(S);//스케줄러 정보 출력
	free(q.list);//q에서 프로세스 정보 저장을 위해 동적할당 한 배열의 메모리 해제
}

//SJF 스케줄링 함수
void SJF_Scheduling(scheduler* S) {
	printf("<SJF 스케줄링>\n[간트차트]\n");
	reset(S);//스케줄러 정보 초기화(ID, 도착시간, 서비스 시간, 우선순위, 프로세스 개수, 타임슬라이스, 인덱스 제외)
	arr_t_sort(S);//도착 시간 기준으로 정렬
	queuetype q;
	process temp;
	int  wt_sum = 0, tt_sum = 0, rt_sum = 0;
	int* check = (int*)malloc(sizeof(int) * S->pro_count);//준비큐에 있거나 있었던 프로세스의 아이디를 저장할 배열
	init_queue(&q, S);//큐 초기화
	int end_pro = 0, cur_time = 0;// end_pro 종료된 프로세스의 수를 저장할 변수, cur_time 현재 시간을 가리키는 변수로 0으로 초기화
	int dup = 0;// 중복된 프로세스가 큐에 있는 경우 1 없는 경우 0을 저장할 변수
	int check_count = 0;//큐에 있거나 있었던 프로세스의 총 개수를 저장할 변수
	int ganttChart = 0;//프로세스의 서비스 시간을 간트차트에 출력하기 위한 변수

	while (S->pro_count != end_pro) {//스케줄러에 있는 프로세스의 개수와 종료된 프로세스의 개수가 같지 않을 때까지 반복
		if (check_count != S->pro_count) {//큐에 들어온 적이 없는 프로세스가 존재하는 경우
			for (int i = 0; i < S->pro_count; i++) {//스케줄러에 있는 프로세스의 개수만큼 반복
				if (S->list[i].arr_t <= cur_time) {//스케줄러에 있는 프로세스 중 선택된 프로세스의 도착시간이 현재 시간보다 작거나 같은 경우
					for (int j = 0; j < S->pro_count; j++)//이미 큐에 들어왔던 프로세스인지 검사하기 위한 반복문
						if (S->list[i].pro_id == check[j])dup = 1;//id가 같은 프로세스가 있다면 dup를 1로 설정
					if (dup == 0) {//dup가 0인 경우는 현재 프로세스가 큐에 들어온 적 없는 프로세스인 경우
						enqueue(&q, S->list[i]);//큐에 삽입
						check_count++;
						check[S->list[i].index] = S->list[i].pro_id;//현재 프로세스의 아이디를 저장
						if (q.count > 1) queue_ser_t_sort(&q);//큐에 2개 이상 있을 경우 SJF이므로 서비스 시간순으로 정렬
					}
					else//현재 프로세스가 큐에 들어왔었던 프로세스인 경우
						dup = 0;
				}
			}
		}
		if (cur_time == 0) temp = dequeue(&q);//현재 시간이 0일 때 첫번째 프로세스를 디큐
		else if (temp.ser_t == 0) {//현재 프로세스가 종료된 상태로 스케줄러에 해당 프로세스의 정보를 계산해서 저장
			printf("[%d] | ", ganttChart); //간트차트에 출력된 서비스 시간 출력
			ganttChart = 0;//끝났으므로 0으로 초기화
			end_pro++;
			S->list[temp.index].wait_t = cur_time - S->list[temp.index].ser_t - temp.arr_t;
			S->list[temp.index].res_t = S->list[temp.index].wait_t + RT;
			S->list[temp.index].turn_a_t = S->list[temp.index].wait_t + S->list[temp.index].ser_t;
			wt_sum += S->list[temp.index].wait_t;
			tt_sum += S->list[temp.index].turn_a_t;
			rt_sum += S->list[temp.index].res_t;
			if (!is_empty(&q))//큐가 비어있지 않은 경우 디큐
				temp = dequeue(&q);
		}
		if (S->pro_count != end_pro) {//모든 프로세스가 종료된 경우에도 출력되는 것을 방지하기 위한 조건문
			printf("P%d", temp.pro_id);//현재 프로세스의 간트차트 출력
			ganttChart++;//간트차트에 출력된 서비스 시간 증가
		}
		cur_time++;//현재 시간 증가
		temp.ser_t--;//현재 프로세스의 서비스 시간 감소
	}
	S->awt = (double)wt_sum / S->pro_count;
	S->att = (double)tt_sum / S->pro_count;
	S->art = (double)rt_sum / S->pro_count;
	print_scheduler(S);
	free(q.list);//동적할당한 배열 메모리 반환
	free(check);//프로세스 아이디를 저장하기 위해 동적할당한 배열의 메모리 반환
}

//HRN 스케줄링 함수
void HRN_Scheduling(scheduler* S) {
	printf("<HRN 스케줄링>\n[간트차트]\n");
	reset(S);//스케줄러 정보 초기화(ID, 도착시간, 서비스 시간, 우선순위, 프로세스 개수, 타임슬라이스, 인덱스 제외)
	arr_t_sort(S);//도착 순서로 정렬
	queuetype q;//큐 생성
	process temp;//큐에서 나온 프로세스의 정보를 저장할 변수
	int  wt_sum = 0, tt_sum = 0, rt_sum = 0, end_pro = 0, cur_time = 0, dup = 0, check_count = 0, ganttChart = 0;//위 함수에 있는 변수와 같은 역할
	int* check = (int*)malloc(sizeof(int) * S->pro_count);//준비큐에 있거나 있었던 프로세스의 아이디를 저장할 배열
	init_queue(&q, S);//큐 초기화

	while (S->pro_count != end_pro) {//스케줄러에 있는 프로세스의 개수와 종료된 프로세스의 개수가 같지 않을 때까지 반복
		if (check_count != S->pro_count) {//큐에 들어온 적이 없는 프로세스가 존재하는 경우
			for (int i = 0; i < S->pro_count; i++) {//스케줄러에 있는 프로세스의 개수만큼 반복
				if (S->list[i].arr_t <= cur_time) {//스케줄러에 있는 프로세스 중 선택된 프로세스의 도착시간이 현재 시간보다 작거나 같은 경우
					for (int j = 0; j < S->pro_count; j++)//이미 큐에 들어왔던 프로세스인지 검사하기 위한 반복문
						if (S->list[i].pro_id == check[j])dup = 1;//id가 같은 프로세스가 있다면 dup를 1로 설정
					if (dup == 0) {//dup가 0인 경우는 현재 프로세스가 큐에 들어온 적 없는 프로세스인 경우
						enqueue(&q, S->list[i]);//큐에 삽입
						if (q.count > 1)//q에 2개 이상의 프로세스가 있다면 정렬
							HRN_Priority(&q, cur_time);//HRN_Priority함수를 이용하여 정렬
						check_count++;
						check[S->list[i].index] = S->list[i].pro_id;//현재 프로세스의 아이디를 저장
					}
					else//현재 프로세스가 큐에 들어왔었던 프로세스인 경우
						dup = 0;
				}
			}
		}
		if (cur_time == 0) temp = dequeue(&q);//현재 시간이 0일 때 첫번째 프로세스를 디큐
		else if (temp.ser_t == 0) {//현재 프로세스가 종료된 상태로 스케줄러에 해당 프로세스의 정보를 계산해서 저장
			printf("[%d] | ", ganttChart);//간트차트에 출력된 서비스 시간 출력
			ganttChart = 0;//끝났으므로 0으로 초기화
			end_pro++;
			S->list[temp.index].wait_t = cur_time - S->list[temp.index].ser_t - temp.arr_t;
			S->list[temp.index].res_t = S->list[temp.index].wait_t + RT;
			S->list[temp.index].turn_a_t = S->list[temp.index].wait_t + S->list[temp.index].ser_t;
			wt_sum += S->list[temp.index].wait_t;
			tt_sum += S->list[temp.index].turn_a_t;
			rt_sum += S->list[temp.index].res_t;
			if (q.count > 1)//큐에 2개 이상의 프로세스가 있다면 정렬
				HRN_Priority(&q, cur_time);//HRN_Priority함수를 이용하여 정렬
			if (!is_empty(&q))//큐가 공백이 아닌 경우
				temp = dequeue(&q);//디큐
		}
		if (S->pro_count != end_pro) {//모든 프로세스가 종료된 경우에도 출력되는 것을 방지하기 위한 조건문
			ganttChart++;//간트차트에 출력된 서비스 시간 증가
			printf("P%d", temp.pro_id);//현재 프로세스의 간트차트 출력
		}
		cur_time++;//현재 시간 증가
		temp.ser_t--;//현재 프로세스의 서비스 시간 감소
	}
	S->awt = (double)wt_sum / S->pro_count;
	S->att = (double)tt_sum / S->pro_count;
	S->art = (double)rt_sum / S->pro_count;
	print_scheduler(S);
	free(q.list);//메모리 반환
	free(check);//메모리 반환
}

//비선점형 우선순위 스케줄링 함수
void Non_Priority_Scheduling(scheduler* S) {
	printf("<비선점형 우선순위 스케줄링>\n[간트차트]\n");
	reset(S);//스케줄러 정보 초기화(ID, 도착시간, 서비스 시간, 우선순위, 프로세스 개수, 타임슬라이스, 인덱스 제외)
	arr_t_sort(S);//도착 순서로 정렬
	queuetype q;//큐 생성
	process temp;//큐에서 나온 프로세스의 정보를 저장할 변수
	int  wt_sum = 0, tt_sum = 0, rt_sum = 0, cur_time = 0, check_count = 0, dup = 0, end_pro = 0, ganttChart = 0;//위 함수에 있는 변수와 같은 역할
	int* check = (int*)malloc(sizeof(int) * S->pro_count);//준비큐에 있거나 있었던 프로세스의 아이디를 저장할 배열
	init_queue(&q, S);//큐 초기화

	while (S->pro_count != end_pro) {//스케줄러에 있는 프로세스의 개수와 종료된 프로세스의 개수가 같지 않을 때까지 반복
		if (check_count != S->pro_count) {//큐에 들어온 적이 없는 프로세스가 존재하는 경우
			for (int i = 0; i < S->pro_count; i++) {//스케줄러에 있는 프로세스의 개수만큼 반복
				if (S->list[i].arr_t <= cur_time) {//스케줄러에 있는 프로세스 중 선택된 프로세스의 도착시간이 현재 시간보다 작거나 같은 경우
					for (int j = 0; j < S->pro_count; j++)//이미 큐에 들어왔던 프로세스인지 검사하기 위한 반복문
						if (S->list[i].pro_id == check[j])dup = 1;//id가 같은 프로세스가 있다면 dup를 1로 설정
					if (dup == 0) {//dup가 0인 경우는 현재 프로세스가 큐에 들어온 적 없는 프로세스인 경우
						enqueue(&q, S->list[i]);//큐에 삽입
						check_count++;
						check[S->list[i].index] = S->list[i].pro_id; // 현재 프로세스의 아이디를 저장
						if (q.count > 1) queue_pro_pri_sort(&q); //큐에 2개 이상의 프로세스가 있다면 우선순위를 기준으로 정렬
					}
					else//현재 프로세스가 큐에 들어왔었던 프로세스인 경우
						dup = 0;
				}
			}
		}
		if (cur_time == 0) temp = dequeue(&q);//현재 시간이 0일 때 첫번째 프로세스를 디큐
		else if (temp.ser_t == 0) {//현재 프로세스가 종료된 상태로 스케줄러에 해당 프로세스의 정보를 계산해서 저장
			printf("[%d] | ", ganttChart);//간트차트에 출력된 서비스 시간 출력
			ganttChart = 0;//끝났으므로 0으로 초기화
			end_pro++;
			S->list[temp.index].wait_t = cur_time - S->list[temp.index].ser_t - temp.arr_t;
			S->list[temp.index].res_t = S->list[temp.index].wait_t + RT;//응답 시간 1로 설정
			S->list[temp.index].turn_a_t = S->list[temp.index].wait_t + S->list[temp.index].ser_t;
			wt_sum += S->list[temp.index].wait_t;
			tt_sum += S->list[temp.index].turn_a_t;
			rt_sum += S->list[temp.index].res_t;
			if (!is_empty(&q))//큐가 공백이 아닌 경우
				temp = dequeue(&q);//디큐
		}
		if (S->pro_count != end_pro) {//모든 프로세스가 종료된 경우에도 출력되는 것을 방지하기 위한 조건문
			printf("P%d", temp.pro_id);//현재 프로세스의 간트차트 출력
			ganttChart++;//간트차트에 출력된 서비스 시간 증가
		}
		cur_time++;//현재 시간 증가
		temp.ser_t--;//현재 프로세스의 서비스 시간 감소
	}
	S->awt = (double)wt_sum / S->pro_count;
	S->att = (double)tt_sum / S->pro_count;
	S->art = (double)rt_sum / S->pro_count;
	print_scheduler(S);//스케줄러 정보 출력
	free(q.list);//메모리 반환
	free(check);//메모리 반환
}

//라운드로빈 스케줄링 함수
void RR_Scheduling(scheduler* S) {
	printf("<Round-Robin 스케줄링>\n[간트차트]\n");
	reset(S);//스케줄러 정보 초기화(ID, 도착시간, 서비스 시간, 우선순위, 프로세스 개수, 타임슬라이스, 인덱스 제외)
	arr_t_sort(S);//도착 순서로 정렬
	queuetype q;//큐 생성
	process temp;//큐에서 나온 프로세스의 정보를 저장할 변수
	int  wt_sum = 0, tt_sum = 0, rt_sum = 0, cur_time = 0, time_slice = 0, check_count = 0, dup = 0, end_pro = 0, ganttChart = 0;
	//time_slice는 주어진 타임슬라이스 시간과 비교하기 위해 현재 프로세스가 사용한 시간을 저장할 변수, 나머지 변수는 위 함수에 있는 변수와 동일한 기능
	int* check = (int*)malloc(sizeof(int) * S->pro_count);//준비큐에 있거나 있었던 프로세스의 아이디를 저장할 배열
	init_queue(&q, S);//큐 초기화

	while (S->pro_count != end_pro) {//스케줄러에 있는 프로세스의 개수와 종료된 프로세스의 개수가 같지 않을 때까지 반복
		if (check_count != S->pro_count) {//큐에 들어온 적이 없는 프로세스가 존재하는 경우
			for (int i = 0; i < S->pro_count; i++) {//스케줄러에 있는 프로세스의 개수만큼 반복
				if (S->list[i].arr_t <= cur_time) {//스케줄러에 있는 프로세스 중 선택된 프로세스의 도착시간이 현재 시간보다 작거나 같은 경우
					for (int j = 0; j < S->pro_count; j++)//이미 큐에 들어왔던 프로세스인지 검사하기 위한 반복문
						if (S->list[i].pro_id == check[j])dup = 1;//id가 같은 프로세스가 있다면 dup를 1로 설정
					if (dup == 0) {//dup가 0인 경우는 현재 프로세스가 큐에 들어온 적 없는 프로세스인 경우
						enqueue(&q, S->list[i]);//큐에 해당 프로세스 삽입
						check_count++;
						check[S->list[i].index] = S->list[i].pro_id; //현재 프로세스의 아이디를 저장
					}
					else//현재 프로세스가 큐에 들어왔었던 프로세스인 경우
						dup = 0;
				}
			}
		}
		if (cur_time == 0) temp = dequeue(&q);//현재 시간이 0일 때 첫번째 프로세스를 디큐

		if (time_slice == S->time_slice && temp.ser_t != 0) {//현재 프로세스가 주어진 타임슬라이스를 다 썼지만 작업이 끝나지 않은 상태
			printf("[%d] | ", ganttChart);//간트차트에 출력된 서비스 시간 출력
			ganttChart = 0;//끝났으므로 0으로 초기화
			time_slice = 0;//다음 프로세스의 시간을 저장하기 위해 0으로 초기화
			enqueue(&q, temp);//작업이 남았으므로 다시 큐에 삽입
			temp = dequeue(&q);//디큐
		}
		else if (temp.ser_t == 0) {//현재 프로세스가 종료된 상태로 스케줄러에 해당 프로세스의 정보를 계산해서 저장
			printf("[%d] | ", ganttChart);//간트차트에 출력된 서비스 시간 출력
			ganttChart = 0;//끝났으므로 0으로 초기화
			end_pro++;
			time_slice = 0; //다음 프로세스의 시간을 저장하기 위해 0으로 초기화
			S->list[temp.index].wait_t = cur_time - S->list[temp.index].ser_t - temp.arr_t;
			S->list[temp.index].turn_a_t = S->list[temp.index].wait_t + S->list[temp.index].ser_t;
			wt_sum += S->list[temp.index].wait_t;
			tt_sum += S->list[temp.index].turn_a_t;
			rt_sum += S->list[temp.index].res_t;
			if (!is_empty(&q))//큐가 공백이 아닌 경우
				temp = dequeue(&q);//디큐
		}
		if (S->pro_count != end_pro) {//모든 프로세스가 종료된 경우에도 출력되는 것을 방지하기 위한 조건문
			printf("P%d", temp.pro_id);//현재 프로세스의 간트차트 출력
			ganttChart++;//간트차트에 출력된 서비스 시간 증가
		}
		temp.count++;//해당 프로세스의 현재 수행시간 증가
		cur_time++;//현재 시간 증가
		if (temp.count == RT)S->list[temp.index].res_t = cur_time - temp.arr_t;//해당 프로세스의 현재 수행시간이 주어진 반응시간과 같으므로 반응시간 입력
		time_slice++;//해당 프로세스의 사용시간 증가
		temp.ser_t--;//해당 프로세스의 서비스 시간 감소

	}
	S->awt = (double)wt_sum / S->pro_count;
	S->att = (double)tt_sum / S->pro_count;
	S->art = (double)rt_sum / S->pro_count;
	print_scheduler(S);//스케줄러의 정보 출력
	free(q.list);//메모리 반환
	free(check);//메모리 반환
}

//SRT 스케줄링 함수
void SRT_Scheduling(scheduler* S) {
	printf("<SRT스케줄링>\n[간트차트]\n");
	reset(S);//스케줄러 정보 초기화(ID, 도착시간, 서비스 시간, 우선순위, 프로세스 개수, 타임슬라이스, 인덱스 제외)
	arr_t_sort(S);//도착 순서로 정렬
	queuetype q;//큐 생성
	process temp;//큐에서 나온 프로세스의 정보를 저장할 변수
	int  wt_sum = 0, tt_sum = 0, rt_sum = 0, cur_time = 0, end_pro = 0, dup = 0, check_count = 0, time_slice = 0, ganttChart = 0;//라운드로빈 함수의 변수와 같은 역할
	int* check = (int*)malloc(sizeof(int) * S->pro_count);//준비큐에 있거나 있었던 프로세스의 아이디를 저장할 배열
	init_queue(&q, S);//큐 초기화

	while (S->pro_count != end_pro) {//스케줄러에 있는 프로세스의 개수와 종료된 프로세스의 개수가 같지 않을 때까지 반복
		if (check_count != S->pro_count) {//큐에 들어온 적이 없는 프로세스가 존재하는 경우
			for (int i = 0; i < S->pro_count; i++) {//스케줄러에 있는 프로세스의 개수만큼 반복
				if (S->list[i].arr_t <= cur_time) {//스케줄러에 있는 프로세스 중 선택된 프로세스의 도착시간이 현재 시간보다 작거나 같은 경우
					for (int j = 0; j < S->pro_count; j++)//이미 큐에 들어왔던 프로세스인지 검사하기 위한 반복문
						if (S->list[i].pro_id == check[j])dup = 1;//id가 같은 프로세스가 있다면 dup를 1로 설정
					if (dup == 0) {//dup가 0인 경우는 현재 프로세스가 큐에 들어온 적 없는 프로세스인 경우
						enqueue(&q, S->list[i]);//큐에 해당 프로세스 삽입
						if (q.count > 1)//큐에 2개 이상의 프로세스가 있는 경우 서비스 시간 기준으로 정렬
							queue_ser_t_sort(&q);
						check_count++;
						check[S->list[i].index] = S->list[i].pro_id;//현재 프로세스의 아이디를 저장
					}
					else//현재 프로세스가 큐에 들어왔었던 프로세스인 경우
						dup = 0;
				}
			}
		}
		if (cur_time == 0)//현재 시간이 0일 때 첫번째 프로세스를 디큐
			temp = dequeue(&q);

		if (time_slice == S->time_slice && temp.ser_t != 0) {//현재 프로세스가 주어진 타임슬라이스를 다 썼지만 작업이 끝나지 않은 상태
			printf("[%d] | ", ganttChart);//간트차트에 출력된 서비스 시간 출력
			ganttChart = 0;//끝났으므로 0으로 초기화
			time_slice = 0;//다음 프로세스의 시간을 저장하기 위해 0으로 초기화
			enqueue(&q, temp);//작업이 남았으므로 다시 큐에 삽입
			if (q.count > 1)//큐에 2개 이상의 프로세스가 있는 경우 서비스 시간 기준으로 정렬
				queue_ser_t_sort(&q);
			temp = dequeue(&q);
		}
		else if (temp.ser_t == 0) {//현재 프로세스가 종료된 상태로 스케줄러에 해당 프로세스의 정보를 계산해서 저장
			printf("[%d] | ", ganttChart);//간트차트에 출력된 서비스 시간 출력
			ganttChart = 0;//끝났으므로 0으로 초기화
			end_pro++;
			time_slice = 0;//다음 프로세스의 시간을 저장하기 위해 0으로 초기화
			S->list[temp.index].wait_t = cur_time - S->list[temp.index].ser_t - temp.arr_t;
			S->list[temp.index].turn_a_t = S->list[temp.index].wait_t + S->list[temp.index].ser_t;
			wt_sum += S->list[temp.index].wait_t;
			tt_sum += S->list[temp.index].turn_a_t;
			rt_sum += S->list[temp.index].res_t;
			if (!is_empty(&q))//큐가 공백이 아닌 경우 디큐
				temp = dequeue(&q);
		}
		if (S->pro_count != end_pro) {//모든 프로세스가 종료된 경우에도 출력되는 것을 방지하기 위한 조건문
			printf("P%d", temp.pro_id);//현재 프로세스의 간트차트 출력
			ganttChart++;//간트차트에 출력된 서비스 시간 증가
		}
		temp.count++;//해당 프로세스의 현재 수행시간 증가
		cur_time++;//현재 시간 증가
		if (temp.count == RT)S->list[temp.index].res_t = cur_time - temp.arr_t;//해당 프로세스의 현재 수행시간이 주어진 반응시간과 같으므로 반응시간 입력
		time_slice++;//해당 프로세스의 사용시간 증가
		temp.ser_t--;//해당 프로세스의 서비스 시간 감소
	}
	S->awt = (double)wt_sum / S->pro_count;
	S->att = (double)tt_sum / S->pro_count;
	S->art = (double)rt_sum / S->pro_count;
	print_scheduler(S);//스케줄러의 정보 출력
	free(q.list);//메모리 반환
	free(check);//메모리 반환
}

//선점형 우선순위 스케줄링 함수
void Priority_Scheduling(scheduler* S) {
	printf("<선점형 우선순위 스케줄링>\n[간트차트]\n");
	reset(S);//스케줄러 정보 초기화(ID, 도착시간, 서비스 시간, 우선순위, 프로세스 개수, 타임슬라이스, 인덱스 제외)
	arr_t_sort(S);//도착 순서로 정렬
	queuetype q;//큐 생성
	process temp;//큐에서 나온 프로세스의 정보를 저장할 변수
	int  wt_sum = 0, tt_sum = 0, rt_sum = 0, cur_time = 0, end_pro = 0, dup = 0, check_count = 0, ganttChart = 0;//라운드로빈의 변수들과 같은 역할
	int* check = (int*)malloc(sizeof(int) * S->pro_count);//준비큐에 있거나 있었던 프로세스의 아이디를 저장할 배열
	init_queue(&q, S);//큐 초기화

	while (S->pro_count != end_pro) {//스케줄러에 있는 프로세스의 개수와 종료된 프로세스의 개수가 같지 않을 때까지 반복
		if (check_count != S->pro_count) {//큐에 들어온 적이 없는 프로세스가 존재하는 경우
			for (int i = 0; i < S->pro_count; i++) {//스케줄러에 있는 프로세스의 개수만큼 반복
				if (S->list[i].arr_t <= cur_time) {//스케줄러에 있는 프로세스 중 선택된 프로세스의 도착시간이 현재 시간보다 작거나 같은 경우
					for (int j = 0; j < S->pro_count; j++)//이미 큐에 들어왔던 프로세스인지 검사하기 위한 반복문
						if (S->list[i].pro_id == check[j])dup = 1;//id가 같은 프로세스가 있다면 dup를 1로 설정
					if (dup == 0) {//dup가 0인 경우는 현재 프로세스가 큐에 들어온 적 없는 프로세스인 경우
						enqueue(&q, S->list[i]);//큐에 해당 프로세스 삽입
						if (q.count > 1)//큐에 프로세스가 2개 이상인 경우 우선순위 기준으로 정렬
							queue_pro_pri_sort(&q);
						check_count++;
						check[S->list[i].index] = S->list[i].pro_id;//현재 프로세스의 아이디를 저장
					}
					else//현재 프로세스가 큐에 들어왔었던 프로세스인 경우
						dup = 0;
				}
			}
		}
		if (cur_time == 0)//현재 시간이 0일 때 첫번째 프로세스를 디큐
			temp = dequeue(&q);

		if (q.count != 0 && temp.ser_t != 0 && temp.pro_pri > q.list[q.front + 1].pro_pri) {//현재 프로세스의 작업 시간이 남아있으며 큐에 프로세스가 1개 이상 있고 제일 앞에 있는 프로세스의 우선 순위가 현재 프로세스의 우선순위보다 높은 경우
			printf("[%d] | ", ganttChart);//간트차트에 출력된 서비스 시간 출력
			ganttChart = 0;//끝났으므로 0으로 초기화
			enqueue(&q, temp);//작업이 남았으므로 다시 큐에 삽입
			if (q.count > 1)//큐에 프로세스가 2개 이상 있는 경우
				queue_pro_pri_sort(&q);//우선순위 기준으로 정렬
			temp = dequeue(&q);//디큐
		}
		else if (temp.ser_t == 0) {//현재 프로세스가 종료된 상태로 스케줄러에 해당 프로세스의 정보를 계산해서 저장
			printf("[%d] | ", ganttChart);//간트차트에 출력된 서비스 시간 출력
			ganttChart = 0;//끝났으므로 0으로 초기화
			end_pro++;
			S->list[temp.index].wait_t = cur_time - S->list[temp.index].ser_t - temp.arr_t;
			S->list[temp.index].turn_a_t = S->list[temp.index].wait_t + S->list[temp.index].ser_t;
			wt_sum += S->list[temp.index].wait_t;
			tt_sum += S->list[temp.index].turn_a_t;
			rt_sum += S->list[temp.index].res_t;
			if (!is_empty(&q))//큐가 공백이 아닌 경우 디큐
				temp = dequeue(&q);
		}
		if (S->pro_count != end_pro) {//모든 프로세스가 종료된 경우에도 출력되는 것을 방지하기 위한 조건문
			printf("P%d", temp.pro_id);//현재 프로세스의 간트차트 출력
			ganttChart++;//간트차트에 출력된 서비스 시간 증가
		}

		temp.count++;//해당 프로세스의 현재 수행시간 증가
		cur_time++;//현재 시간 증가
		if (temp.count == RT)S->list[temp.index].res_t = cur_time - temp.arr_t;//해당 프로세스의 현재 수행시간이 주어진 반응시간과 같으므로 반응시간 입력
		temp.ser_t--;//해당 프로세스의 서비스 시간 감소
	}
	S->awt = (double)wt_sum / S->pro_count;
	S->att = (double)tt_sum / S->pro_count;
	S->art = (double)rt_sum / S->pro_count;
	print_scheduler(S);//스케줄러의 정보 출력
	free(q.list);//메모리 반환
	free(check);//메모리 반환
}

//프로그램 동작 함수
void run(scheduler* S) {
	int menu;//사용자가 입력한 메뉴를 저장할 변수
	print_process_file(S);//파일에서 읽어온 프로세스의 정보를 출력
	while (1) {//무한 반복문
		print_menu();//메뉴 출력
		printf("메뉴를 선택해주세요: ");
		scanf("%d", &menu);
		getchar();//공백문자 제거
		system("cls");//콘솔 화면 지우기
		switch (menu)
		{
		case 1://1인 경우 FCFS 스케줄링 호출
			FCFS_Scheduling(S);
			break;
		case 2://2인 경우 SJF 스케줄링 호출
			SJF_Scheduling(S);
			break;
		case 3://3인 경우 HRN 스케줄링 호출
			HRN_Scheduling(S);
			break;
		case 4://4인 경우 비선점형 우선순위 스케줄링 호출
			Non_Priority_Scheduling(S);
			break;
		case 5://5인 경우 라운드로빈 스케줄링 호출
			RR_Scheduling(S);
			break;
		case 6://6인 경우 SRT 스케줄링 호출
			SRT_Scheduling(S);
			break;
		case 7://7인 경우 선점형 우선순위 스케줄링 호출
			Priority_Scheduling(S);
			break;
		case 8://메모리 해제 후 종료
			free(S->list);//메모리 해제
			exit(1);
		default:
			print_process_file(S);//다른 숫자 입력시 파일에서 읽어온 프로세스의 정보 재출력
		}
	}
}

int main() {
	scheduler S;//스케줄러 생성
	int index = 0;//프로세스가 스케줄러의 배열에 저장된 인덱스를 
	char a;

	FILE* fp = fopen("data.txt", "r");//파일을 읽기 모드로 오픈
	if (fp == NULL) {//파일 포인터가 NULL인 경우 파일 열기 오류 문구 출력 후 종료
		printf("파일 오류\n");
		return 0;
	}

	fscanf(fp, "%d", &S.pro_count);//파일의 첫 번째 숫자를 가져와 저장
	S.list = (process*)malloc(sizeof(process) * S.pro_count);//프로세스의 수만큼 list배열에 메모리 동적 할당
	if (S.list == NULL) {//list가 NULL인 경우 메모리 할당 오류
		printf("동적 할당 오류\n");
		return 0;
	}
	for (int i = 0; i < S.pro_count; i++) {//프로세스 개수 만큼 반복하여 파일에 있는 프로세스 정보 저장
		fscanf(fp, "%c", &a);
		fscanf(fp, "%c", &a);
		fscanf(fp, "%d", &S.list[i].pro_id);
		fscanf(fp, "%d", &S.list[i].arr_t);
		fscanf(fp, "%d", &S.list[i].ser_t);
		fscanf(fp, "%d", &S.list[i].pro_pri);
		S.list[i].index = i;
	}
	fscanf(fp, "%d", &S.time_slice);//타임 슬라이스를 읽어와 저장
	fclose(fp);//파일 닫기
	run(&S);
	return 0;
}