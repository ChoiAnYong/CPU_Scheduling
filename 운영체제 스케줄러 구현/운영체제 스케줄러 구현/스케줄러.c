#include<stdio.h>
#include<stdlib.h>
#include<Windows.h>
#pragma warning(disable:4996)

const int RT = 1;//����ð�

//���μ����� ������ ���� ����ü
typedef struct process {
	int pro_id, arr_t, ser_t, pro_pri, res_t, wait_t, turn_a_t, index, count;//���μ��� ID, ���� �ð�, ���� �ð�, �켱 ����, ���� �ð�, ��� �ð�, ��ȯ �ð�, ����� �迭�� �ε���, ���� ����ð�
	double HRN_pro_pri;//HRN���� ���� �켱������ �������� �ƴ� �Ǽ����� �����Ƿ� �Ǽ��� �켱������ �����ϱ� ���� ����
}process;

//�����ٷ��� ������ ���� ����ü
typedef struct scheduler {
	int pro_count, time_slice;//���μ��� ����, Ÿ�ӽ����̽�
	double awt, art, att;//��� ���ð�, ��� ����ð�, ��� ��ȯ�ð�
	process* list;//���μ����� ������ �迭
}scheduler;

//����ť�� �غ�ť�� �����ϱ� ���� ����ü
typedef struct {
	int size;//ť�� ũ��
	int rear;//ť�� ������ �ε���
	int front;//ť�� ù ��° �ε���(����ť�̹Ƿ� ť�� ù ��° ���� �ε����� �ƴ�) 
	int count;//���� ť�� �ִ� ���μ����� ����
	process* list;//ť�� ���� ���μ����� ������ ������ �迭
}queuetype;

//ť �ʱ�ȭ �Լ�
void init_queue(queuetype* q, scheduler* S) {
	q->front = 0;//front�� 0���� �ʱ�ȭ
	q->rear = 0;//rear�� 0���� �ʱ�ȭ
	q->count = 0;//���� ť�� �ִ� ���μ����� ������ 0���� �ʱ�ȭ 
	q->size = S->pro_count + 1;//����ť�̹Ƿ� ť�� ũ�⸦ �����ٷ��� �ִ� ���μ����� ���� +1�� �ʱ�ȭ
	q->list = (process*)malloc(sizeof(process) * q->size);//���μ����� ������ ������ �迭�� ť�� ũ�⸸ŭ ���� �Ҵ�
}

//ť ���� �˻� �Լ�
int is_empty(queuetype* q) {
	return (q->rear == q->front);
}

//ť ��ȭ �˻� �Լ�
int is_full(queuetype* q) {
	return ((q->rear + 1) % q->size == q->front);
}

//ť ��ť �Լ�
void enqueue(queuetype* q, process p) {
	if (is_full(q)) {
		fprintf(stderr, "ť�� ��ȭ���� �Դϴ�.\n");
		return;
	}

	q->rear = (q->rear + 1) % q->size;
	q->list[q->rear] = p;
	q->count++;//ť�� ���μ����� �������Ƿ� count ����
}

//ť ��ť �Լ�
process dequeue(queuetype* q) {
	if (is_empty(q)) {
		fprintf(stderr, "ť�� ��������Դϴ�\n");
		return;
	}

	q->front = (q->front + 1) % q->size;
	q->count--;//ť���� ���μ����� �������Ƿ� count ����
	return q->list[q->front];
}

//�غ�ť�� ���� ���μ����� ���� �ð� �������� �����ϴ� �Լ�
void queue_ser_t_sort(queuetype* q) {
	process temp;//�����ϱ� ���� ��� ���μ����� ������ ������ ����

	//��������
	for (int i = q->count; i > 0; i--) {
		for (int j = 0; j < i - 1; j++) {
			//���� ���μ����� ���� �ð��� ���� ���μ����� ���� �ð����� �� ��� ����
			if (q->list[(q->front + 1 + j) % q->size].ser_t > q->list[(q->front + 2 + j) % q->size].ser_t) {
				temp = q->list[(q->front + 2 + j) % q->size];
				q->list[(q->front + 2 + j) % q->size] = q->list[(q->front + 1 + j) % q->size];
				q->list[(q->front + 1 + j) % q->size] = temp;
			}
			//���� ���μ����� ���� �ð��� ���� ���μ����� ���� �ð��� ���� ��� id������ ����
			else if (q->list[(q->front + 1 + j) % q->size].ser_t == q->list[(q->front + 2 + j) % q->size].ser_t && q->list[(q->front + 1 + j) % q->size].pro_id > q->list[(q->front + 2 + j) % q->size].pro_id) {
				temp = q->list[(q->front + 2 + j) % q->size];
				q->list[(q->front + 2 + j) % q->size] = q->list[(q->front + 1 + j) % q->size];
				q->list[(q->front + 1 + j) % q->size] = temp;
			}
		}
	}
}

//�غ�ť�� ���� ���μ����� �켱���� �������� �����ϴ� �Լ�
void queue_pro_pri_sort(queuetype* q) {
	process temp;//�����ϱ� ���� ��� ���μ����� ������ ������ ����

	//��������
	for (int i = q->count; i > 0; i--) {
		for (int j = 0; j < i - 1; j++) {
			//���� ���μ����� �켱������ ���� ���μ������� ���� ����(���ڰ� �������� �켱������ ����)
			if (q->list[(q->front + 1 + j) % q->size].pro_pri > q->list[(q->front + 2 + j) % q->size].pro_pri) {
				temp = q->list[(q->front + 2 + j) % q->size];
				q->list[(q->front + 2 + j) % q->size] = q->list[(q->front + 1 + j) % q->size];
				q->list[(q->front + 1 + j) % q->size] = temp;
			}
			//���� ���μ����� �켱������ ���� ���μ����� �켱������ ���� ��� id������ ����
			else if (q->list[(q->front + 1 + j) % q->size].pro_pri == q->list[(q->front + 2 + j) % q->size].pro_pri && q->list[(q->front + 1 + j) % q->size].pro_id > q->list[(q->front + 2 + j) % q->size].pro_id) {
				temp = q->list[(q->front + 2 + j) % q->size];
				q->list[(q->front + 2 + j) % q->size] = q->list[(q->front + 1 + j) % q->size];
				q->list[(q->front + 1 + j) % q->size] = temp;
			}
		}
	}
}

//�����ٷ��� ���� ���μ����� ���������� �����ϴ� �Լ�
void arr_t_sort(scheduler* S) {
	process temp;//�����ϱ� ���� ��� ���μ����� ������ ������ ����
	int index_temp;//������ �� ����� �迭�� �ε����� �ٲٱ� ���� �ε��� ������ ������ ����
	//��������
	for (int i = S->pro_count; i > 0; i--) {
		for (int j = 0; j < i - 1; j++) {
			//���� ���μ����� �����ð��� ���� ���μ������� ���� ��� ����
			if (S->list[j].arr_t > S->list[j + 1].arr_t) {
				temp = S->list[j + 1];
				S->list[j + 1] = S->list[j];
				S->list[j] = temp;
				//�Ʒ� ������ ���ϸ� ���ĵǾ� ����� �ε����� ���μ��� ������ �ִ� �ε��� �������� ������� ����
				index_temp = S->list[j].index;
				S->list[j].index = S->list[j + 1].index;
				S->list[j + 1].index = index_temp;
			}
			//�����ð��� ���� ��� id������ ����
			else if (S->list[j].arr_t == S->list[j + 1].arr_t && S->list[j].pro_id > S->list[j + 1].pro_id) {
				temp = S->list[j + 1];
				S->list[j + 1] = S->list[j];
				S->list[j] = temp;
				//�Ʒ� ������ ���ϸ� ���ĵǾ� ����� �ε����� ���μ��� ������ �ִ� �ε��� �������� ������� ����
				index_temp = S->list[j].index;
				S->list[j].index = S->list[j + 1].index;
				S->list[j + 1].index = index_temp;
			}
		}
	}
}

//�����ٷ��� ���� ���μ����� id������ �����ϴ� �Լ�
void pro_id_sort(scheduler* S) {
	process temp;
	int index_temp;
	//��������
	for (int i = S->pro_count; i > 0; i--) {
		for (int j = 0; j < i - 1; j++) {
			if (S->list[j].pro_id > S->list[j + 1].pro_id) {
				temp = S->list[j + 1];
				S->list[j + 1] = S->list[j];
				S->list[j] = temp;
				//�Ʒ� ������ ���ϸ� ���ĵǾ� ����� �ε����� ���μ��� ������ �ִ� �ε��� �������� ������� ����
				index_temp = S->list[j].index;
				S->list[j].index = S->list[j + 1].index;
				S->list[j + 1].index = index_temp;
			}
		}
	}
}

//HRN �����ٸ����� �غ�ť�� �ִ� ���μ������� �켱 ������ ���ϰ� �������ִ� �Լ�
void HRN_Priority(queuetype* q, int time) {
	process temp;
	//���� �ð� �������� ť�� �ִ� ���μ����� �켱������ ���
	for (int i = 0; i < q->count; i++) {
		q->list[q->front + 1 + i].HRN_pro_pri = (double)(time - q->list[q->front + 1 + i].arr_t + q->list[q->front + 1 + i].ser_t) / q->list[q->front + 1 + i].ser_t;
	}
	//��������
	for (int i = q->count; i > 0; i--) {
		for (int j = 0; j < i - 1; j++) {
			//���� ���μ����� HRN �켱������ ���� ���μ����� HRN �켱�������� ���� ���(���ڰ� Ŭ���� �켱������ ����)
			if (q->list[(q->front + 1 + j) % q->size].HRN_pro_pri < q->list[(q->front + 2 + j) % q->size].HRN_pro_pri) {
				temp = q->list[(q->front + 2 + j) % q->size];
				q->list[(q->front + 2 + j) % q->size] = q->list[(q->front + 1 + j) % q->size];
				q->list[(q->front + 1 + j) % q->size] = temp;
			}
			//�켱������ ���� ��� id������ ����
			else if (q->list[(q->front + 1 + j) % q->size].HRN_pro_pri == q->list[(q->front + 2 + j) % q->size].HRN_pro_pri && q->list[(q->front + 1 + j) % q->size].pro_id > q->list[(q->front + 2 + j) % q->size].pro_id) {
				temp = q->list[(q->front + 2 + j) % q->size];
				q->list[(q->front + 2 + j) % q->size] = q->list[(q->front + 1 + j) % q->size];
				q->list[(q->front + 1 + j) % q->size] = temp;
			}
		}
	}
}

//�����ٷ��� ������ ������ִ� �Լ�(���ð�, ����ð�, ��ȯ �ð�)
void print_scheduler(scheduler* S) {
	//id������ ����ϱ� ���� pro_id_sort�Լ� ȣ��
	pro_id_sort(S);//�����ٷ��� �ִ� ���μ����� id������ ����
	printf("\n����������������������������������������\n");
	printf("�� ID ��   ���ð�  ��\n");
	printf("����������������������������������������\n");
	for (int i = 0; i < S->pro_count; i++)
		printf("��P%-2d ��     %3d     ��\n", S->list[i].pro_id, S->list[i].wait_t);
	printf("����������������������������������������\n");
	printf("����զ�    ");
	if ((int)S->awt / 10 == 0)printf(" ");//����� ���ڸ� ���� ǥ�� ������ ��츦 �����ϱ� ���� ���ǹ�
	printf("%.2f    ��\n", S->awt);//�Ҽ��� 2�ڸ������� ���
	printf("����������������������������������������");
	printf("\n����������������������������������������\n");
	printf("�� ID ��   ����ð�  ��\n");
	printf("����������������������������������������\n");
	for (int i = 0; i < S->pro_count; i++)
		printf("��P%-2d ��     %3d     ��\n", S->list[i].pro_id, S->list[i].res_t);
	printf("����������������������������������������\n");
	printf("����զ�    ");
	if ((int)S->art / 10 == 0)printf(" ");//����� ���ڸ� ���� ǥ�� ������ ��츦 �����ϱ� ���� ���ǹ�
	printf("%.2f    ��\n", S->art);//�Ҽ��� 2�ڸ������� ���	
	printf("����������������������������������������");
	printf("\n����������������������������������������\n");
	printf("�� ID ��   ��ȯ�ð�  ��\n");
	printf("����������������������������������������\n");
	for (int i = 0; i < S->pro_count; i++)
		printf("��P%-2d ��     %3d     ��\n", S->list[i].pro_id, S->list[i].turn_a_t);
	printf("����������������������������������������\n");
	printf("����զ�    ");
	if ((int)S->awt / 10 == 0)printf(" ");//����� ���ڸ� ���� ǥ�� ������ ��츦 �����ϱ� ���� ���ǹ�
	printf("%.2f    ��\n", S->att);//�Ҽ��� 2�ڸ������� ���
	printf("����������������������������������������\n\n");
}

//�޴� ��� �Լ�
void print_menu() {
	printf("==================MENU==================\n");
	printf("1. FCFS �����ٸ�\n");
	printf("2. SJF �����ٸ�\n");
	printf("3. HRN �����ٸ�\n");
	printf("4. ������ �켱���� �����ٸ�\n");
	printf("5. ����κ� �����ٸ�\n");
	printf("6. SRT �����ٸ�\n");
	printf("7. ������ �켱���� �����ٸ�\n");
	printf("8. ����\n");
	printf("========================================\n");
}

//���Ͽ��� �о�� ���μ��� ������ ������ִ� �Լ�
void print_process_file(scheduler* S) {
	pro_id_sort(S);
	printf("\t    <���μ��� ����>");
	printf("\n����������������������������������������������������������������������������������\n");
	printf("�� ID �� �����ð� �� ���񽺽ð� �� �켱���� ��\n");
	printf("����������������������������������������������������������������������������������\n");
	for (int i = 0; i < S->pro_count; i++)
		printf("��P%-2d ��   %3d    ��    %3d     ��   %3d    ��\n", S->list[i].pro_id, S->list[i].arr_t, S->list[i].ser_t, S->list[i].pro_pri);
	printf("����������������������������������������������������������������������������������\n");
	printf("\t   [Ÿ�� �����̽� %d]\n", S->time_slice);
}

//�����ٷ� ���� �ʱ�ȭ �Լ�(ID, �����ð�, ���� �ð�, �켱����, ���μ��� ����, Ÿ�ӽ����̽�, �ε��� ����)
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

//FCFS �����ٸ� �Լ�
void FCFS_Scheduling(scheduler* S) {
	printf("<FCFS �����ٸ�>\n[��Ʈ��Ʈ]\n");
	reset(S);//�����ٷ� ���� �ʱ�ȭ(ID, �����ð�, ���� �ð�, �켱����, ���μ��� ����, Ÿ�ӽ����̽�, �ε��� ����)
	arr_t_sort(S);//���� ������ ����
	queuetype q;//ť ����
	process temp;//ť���� ���� ���μ����� ������ ������ ����
	int run_time = 0, wt_sum = 0, tt_sum = 0, rt_sum = 0;//���� �ð�, ���ð� ��, ��ȯ�ð� ��, ����ð� ���� ������ ����

	init_queue(&q, S);//ť �ʱ�ȭ
	for (int i = 0; i < S->pro_count; i++) {
		enqueue(&q, S->list[i]);//���������� ť�� ����
	}

	for (int i = 0; i < S->pro_count; i++) {
		temp = dequeue(&q);//�ϳ��� ��ť
		S->list[temp.index].wait_t = run_time - temp.arr_t;//���� ���μ����� ���ð� ��� �� ����
		S->list[temp.index].res_t = S->list[temp.index].wait_t + RT;//���� ���μ����� �����ð� ��� �� ����
		S->list[temp.index].turn_a_t = S->list[temp.index].wait_t + temp.ser_t;//���� ���μ����� ��ȯ�ð� ��� �� ����

		for (int i = 0; i < temp.ser_t; i++)//���� ���μ����� ���� �ð���ŭ ��Ʈ��Ʈ ���
			printf("P%d", temp.pro_id);
		printf("[%d] | ", temp.ser_t);//���� ���μ����� �����ϰ� ���� �ð��� ��Ÿ���� ���� ��¹�
		run_time += temp.ser_t;//����ð��� ���� ���μ����� ���� �ð���ŭ ����
		wt_sum += S->list[temp.index].wait_t;//���μ������� ���ð� ���� ���ϱ� ���� ���� ���μ����� ���ð� ���ϱ�
		tt_sum += S->list[temp.index].turn_a_t;//���μ������� ��ȯ�ð� ���� ���ϱ� ���� ���� ���μ����� ��ȯ�ð� ���ϱ�
		rt_sum += S->list[temp.index].res_t;//���μ������� ����ð� ���� ���ϱ� ���� ���� ���μ����� ����ð� ���ϱ�
	}
	S->awt = (double)wt_sum / S->pro_count;//��� ���μ����� �������Ƿ� ����� ���ؼ� �����ٷ��� ����
	S->att = (double)tt_sum / S->pro_count;//��� ���μ����� �������Ƿ� ����� ���ؼ� �����ٷ��� ����
	S->art = (double)rt_sum / S->pro_count;//��� ���μ����� �������Ƿ� ����� ���ؼ� �����ٷ��� ����
	print_scheduler(S);//�����ٷ� ���� ���
	free(q.list);//q���� ���μ��� ���� ������ ���� �����Ҵ� �� �迭�� �޸� ����
}

//SJF �����ٸ� �Լ�
void SJF_Scheduling(scheduler* S) {
	printf("<SJF �����ٸ�>\n[��Ʈ��Ʈ]\n");
	reset(S);//�����ٷ� ���� �ʱ�ȭ(ID, �����ð�, ���� �ð�, �켱����, ���μ��� ����, Ÿ�ӽ����̽�, �ε��� ����)
	arr_t_sort(S);//���� �ð� �������� ����
	queuetype q;
	process temp;
	int  wt_sum = 0, tt_sum = 0, rt_sum = 0;
	int* check = (int*)malloc(sizeof(int) * S->pro_count);//�غ�ť�� �ְų� �־��� ���μ����� ���̵� ������ �迭
	init_queue(&q, S);//ť �ʱ�ȭ
	int end_pro = 0, cur_time = 0;// end_pro ����� ���μ����� ���� ������ ����, cur_time ���� �ð��� ����Ű�� ������ 0���� �ʱ�ȭ
	int dup = 0;// �ߺ��� ���μ����� ť�� �ִ� ��� 1 ���� ��� 0�� ������ ����
	int check_count = 0;//ť�� �ְų� �־��� ���μ����� �� ������ ������ ����
	int ganttChart = 0;//���μ����� ���� �ð��� ��Ʈ��Ʈ�� ����ϱ� ���� ����

	while (S->pro_count != end_pro) {//�����ٷ��� �ִ� ���μ����� ������ ����� ���μ����� ������ ���� ���� ������ �ݺ�
		if (check_count != S->pro_count) {//ť�� ���� ���� ���� ���μ����� �����ϴ� ���
			for (int i = 0; i < S->pro_count; i++) {//�����ٷ��� �ִ� ���μ����� ������ŭ �ݺ�
				if (S->list[i].arr_t <= cur_time) {//�����ٷ��� �ִ� ���μ��� �� ���õ� ���μ����� �����ð��� ���� �ð����� �۰ų� ���� ���
					for (int j = 0; j < S->pro_count; j++)//�̹� ť�� ���Դ� ���μ������� �˻��ϱ� ���� �ݺ���
						if (S->list[i].pro_id == check[j])dup = 1;//id�� ���� ���μ����� �ִٸ� dup�� 1�� ����
					if (dup == 0) {//dup�� 0�� ���� ���� ���μ����� ť�� ���� �� ���� ���μ����� ���
						enqueue(&q, S->list[i]);//ť�� ����
						check_count++;
						check[S->list[i].index] = S->list[i].pro_id;//���� ���μ����� ���̵� ����
						if (q.count > 1) queue_ser_t_sort(&q);//ť�� 2�� �̻� ���� ��� SJF�̹Ƿ� ���� �ð������� ����
					}
					else//���� ���μ����� ť�� ���Ծ��� ���μ����� ���
						dup = 0;
				}
			}
		}
		if (cur_time == 0) temp = dequeue(&q);//���� �ð��� 0�� �� ù��° ���μ����� ��ť
		else if (temp.ser_t == 0) {//���� ���μ����� ����� ���·� �����ٷ��� �ش� ���μ����� ������ ����ؼ� ����
			printf("[%d] | ", ganttChart); //��Ʈ��Ʈ�� ��µ� ���� �ð� ���
			ganttChart = 0;//�������Ƿ� 0���� �ʱ�ȭ
			end_pro++;
			S->list[temp.index].wait_t = cur_time - S->list[temp.index].ser_t - temp.arr_t;
			S->list[temp.index].res_t = S->list[temp.index].wait_t + RT;
			S->list[temp.index].turn_a_t = S->list[temp.index].wait_t + S->list[temp.index].ser_t;
			wt_sum += S->list[temp.index].wait_t;
			tt_sum += S->list[temp.index].turn_a_t;
			rt_sum += S->list[temp.index].res_t;
			if (!is_empty(&q))//ť�� ������� ���� ��� ��ť
				temp = dequeue(&q);
		}
		if (S->pro_count != end_pro) {//��� ���μ����� ����� ��쿡�� ��µǴ� ���� �����ϱ� ���� ���ǹ�
			printf("P%d", temp.pro_id);//���� ���μ����� ��Ʈ��Ʈ ���
			ganttChart++;//��Ʈ��Ʈ�� ��µ� ���� �ð� ����
		}
		cur_time++;//���� �ð� ����
		temp.ser_t--;//���� ���μ����� ���� �ð� ����
	}
	S->awt = (double)wt_sum / S->pro_count;
	S->att = (double)tt_sum / S->pro_count;
	S->art = (double)rt_sum / S->pro_count;
	print_scheduler(S);
	free(q.list);//�����Ҵ��� �迭 �޸� ��ȯ
	free(check);//���μ��� ���̵� �����ϱ� ���� �����Ҵ��� �迭�� �޸� ��ȯ
}

//HRN �����ٸ� �Լ�
void HRN_Scheduling(scheduler* S) {
	printf("<HRN �����ٸ�>\n[��Ʈ��Ʈ]\n");
	reset(S);//�����ٷ� ���� �ʱ�ȭ(ID, �����ð�, ���� �ð�, �켱����, ���μ��� ����, Ÿ�ӽ����̽�, �ε��� ����)
	arr_t_sort(S);//���� ������ ����
	queuetype q;//ť ����
	process temp;//ť���� ���� ���μ����� ������ ������ ����
	int  wt_sum = 0, tt_sum = 0, rt_sum = 0, end_pro = 0, cur_time = 0, dup = 0, check_count = 0, ganttChart = 0;//�� �Լ��� �ִ� ������ ���� ����
	int* check = (int*)malloc(sizeof(int) * S->pro_count);//�غ�ť�� �ְų� �־��� ���μ����� ���̵� ������ �迭
	init_queue(&q, S);//ť �ʱ�ȭ

	while (S->pro_count != end_pro) {//�����ٷ��� �ִ� ���μ����� ������ ����� ���μ����� ������ ���� ���� ������ �ݺ�
		if (check_count != S->pro_count) {//ť�� ���� ���� ���� ���μ����� �����ϴ� ���
			for (int i = 0; i < S->pro_count; i++) {//�����ٷ��� �ִ� ���μ����� ������ŭ �ݺ�
				if (S->list[i].arr_t <= cur_time) {//�����ٷ��� �ִ� ���μ��� �� ���õ� ���μ����� �����ð��� ���� �ð����� �۰ų� ���� ���
					for (int j = 0; j < S->pro_count; j++)//�̹� ť�� ���Դ� ���μ������� �˻��ϱ� ���� �ݺ���
						if (S->list[i].pro_id == check[j])dup = 1;//id�� ���� ���μ����� �ִٸ� dup�� 1�� ����
					if (dup == 0) {//dup�� 0�� ���� ���� ���μ����� ť�� ���� �� ���� ���μ����� ���
						enqueue(&q, S->list[i]);//ť�� ����
						if (q.count > 1)//q�� 2�� �̻��� ���μ����� �ִٸ� ����
							HRN_Priority(&q, cur_time);//HRN_Priority�Լ��� �̿��Ͽ� ����
						check_count++;
						check[S->list[i].index] = S->list[i].pro_id;//���� ���μ����� ���̵� ����
					}
					else//���� ���μ����� ť�� ���Ծ��� ���μ����� ���
						dup = 0;
				}
			}
		}
		if (cur_time == 0) temp = dequeue(&q);//���� �ð��� 0�� �� ù��° ���μ����� ��ť
		else if (temp.ser_t == 0) {//���� ���μ����� ����� ���·� �����ٷ��� �ش� ���μ����� ������ ����ؼ� ����
			printf("[%d] | ", ganttChart);//��Ʈ��Ʈ�� ��µ� ���� �ð� ���
			ganttChart = 0;//�������Ƿ� 0���� �ʱ�ȭ
			end_pro++;
			S->list[temp.index].wait_t = cur_time - S->list[temp.index].ser_t - temp.arr_t;
			S->list[temp.index].res_t = S->list[temp.index].wait_t + RT;
			S->list[temp.index].turn_a_t = S->list[temp.index].wait_t + S->list[temp.index].ser_t;
			wt_sum += S->list[temp.index].wait_t;
			tt_sum += S->list[temp.index].turn_a_t;
			rt_sum += S->list[temp.index].res_t;
			if (q.count > 1)//ť�� 2�� �̻��� ���μ����� �ִٸ� ����
				HRN_Priority(&q, cur_time);//HRN_Priority�Լ��� �̿��Ͽ� ����
			if (!is_empty(&q))//ť�� ������ �ƴ� ���
				temp = dequeue(&q);//��ť
		}
		if (S->pro_count != end_pro) {//��� ���μ����� ����� ��쿡�� ��µǴ� ���� �����ϱ� ���� ���ǹ�
			ganttChart++;//��Ʈ��Ʈ�� ��µ� ���� �ð� ����
			printf("P%d", temp.pro_id);//���� ���μ����� ��Ʈ��Ʈ ���
		}
		cur_time++;//���� �ð� ����
		temp.ser_t--;//���� ���μ����� ���� �ð� ����
	}
	S->awt = (double)wt_sum / S->pro_count;
	S->att = (double)tt_sum / S->pro_count;
	S->art = (double)rt_sum / S->pro_count;
	print_scheduler(S);
	free(q.list);//�޸� ��ȯ
	free(check);//�޸� ��ȯ
}

//������ �켱���� �����ٸ� �Լ�
void Non_Priority_Scheduling(scheduler* S) {
	printf("<������ �켱���� �����ٸ�>\n[��Ʈ��Ʈ]\n");
	reset(S);//�����ٷ� ���� �ʱ�ȭ(ID, �����ð�, ���� �ð�, �켱����, ���μ��� ����, Ÿ�ӽ����̽�, �ε��� ����)
	arr_t_sort(S);//���� ������ ����
	queuetype q;//ť ����
	process temp;//ť���� ���� ���μ����� ������ ������ ����
	int  wt_sum = 0, tt_sum = 0, rt_sum = 0, cur_time = 0, check_count = 0, dup = 0, end_pro = 0, ganttChart = 0;//�� �Լ��� �ִ� ������ ���� ����
	int* check = (int*)malloc(sizeof(int) * S->pro_count);//�غ�ť�� �ְų� �־��� ���μ����� ���̵� ������ �迭
	init_queue(&q, S);//ť �ʱ�ȭ

	while (S->pro_count != end_pro) {//�����ٷ��� �ִ� ���μ����� ������ ����� ���μ����� ������ ���� ���� ������ �ݺ�
		if (check_count != S->pro_count) {//ť�� ���� ���� ���� ���μ����� �����ϴ� ���
			for (int i = 0; i < S->pro_count; i++) {//�����ٷ��� �ִ� ���μ����� ������ŭ �ݺ�
				if (S->list[i].arr_t <= cur_time) {//�����ٷ��� �ִ� ���μ��� �� ���õ� ���μ����� �����ð��� ���� �ð����� �۰ų� ���� ���
					for (int j = 0; j < S->pro_count; j++)//�̹� ť�� ���Դ� ���μ������� �˻��ϱ� ���� �ݺ���
						if (S->list[i].pro_id == check[j])dup = 1;//id�� ���� ���μ����� �ִٸ� dup�� 1�� ����
					if (dup == 0) {//dup�� 0�� ���� ���� ���μ����� ť�� ���� �� ���� ���μ����� ���
						enqueue(&q, S->list[i]);//ť�� ����
						check_count++;
						check[S->list[i].index] = S->list[i].pro_id; // ���� ���μ����� ���̵� ����
						if (q.count > 1) queue_pro_pri_sort(&q); //ť�� 2�� �̻��� ���μ����� �ִٸ� �켱������ �������� ����
					}
					else//���� ���μ����� ť�� ���Ծ��� ���μ����� ���
						dup = 0;
				}
			}
		}
		if (cur_time == 0) temp = dequeue(&q);//���� �ð��� 0�� �� ù��° ���μ����� ��ť
		else if (temp.ser_t == 0) {//���� ���μ����� ����� ���·� �����ٷ��� �ش� ���μ����� ������ ����ؼ� ����
			printf("[%d] | ", ganttChart);//��Ʈ��Ʈ�� ��µ� ���� �ð� ���
			ganttChart = 0;//�������Ƿ� 0���� �ʱ�ȭ
			end_pro++;
			S->list[temp.index].wait_t = cur_time - S->list[temp.index].ser_t - temp.arr_t;
			S->list[temp.index].res_t = S->list[temp.index].wait_t + RT;//���� �ð� 1�� ����
			S->list[temp.index].turn_a_t = S->list[temp.index].wait_t + S->list[temp.index].ser_t;
			wt_sum += S->list[temp.index].wait_t;
			tt_sum += S->list[temp.index].turn_a_t;
			rt_sum += S->list[temp.index].res_t;
			if (!is_empty(&q))//ť�� ������ �ƴ� ���
				temp = dequeue(&q);//��ť
		}
		if (S->pro_count != end_pro) {//��� ���μ����� ����� ��쿡�� ��µǴ� ���� �����ϱ� ���� ���ǹ�
			printf("P%d", temp.pro_id);//���� ���μ����� ��Ʈ��Ʈ ���
			ganttChart++;//��Ʈ��Ʈ�� ��µ� ���� �ð� ����
		}
		cur_time++;//���� �ð� ����
		temp.ser_t--;//���� ���μ����� ���� �ð� ����
	}
	S->awt = (double)wt_sum / S->pro_count;
	S->att = (double)tt_sum / S->pro_count;
	S->art = (double)rt_sum / S->pro_count;
	print_scheduler(S);//�����ٷ� ���� ���
	free(q.list);//�޸� ��ȯ
	free(check);//�޸� ��ȯ
}

//����κ� �����ٸ� �Լ�
void RR_Scheduling(scheduler* S) {
	printf("<Round-Robin �����ٸ�>\n[��Ʈ��Ʈ]\n");
	reset(S);//�����ٷ� ���� �ʱ�ȭ(ID, �����ð�, ���� �ð�, �켱����, ���μ��� ����, Ÿ�ӽ����̽�, �ε��� ����)
	arr_t_sort(S);//���� ������ ����
	queuetype q;//ť ����
	process temp;//ť���� ���� ���μ����� ������ ������ ����
	int  wt_sum = 0, tt_sum = 0, rt_sum = 0, cur_time = 0, time_slice = 0, check_count = 0, dup = 0, end_pro = 0, ganttChart = 0;
	//time_slice�� �־��� Ÿ�ӽ����̽� �ð��� ���ϱ� ���� ���� ���μ����� ����� �ð��� ������ ����, ������ ������ �� �Լ��� �ִ� ������ ������ ���
	int* check = (int*)malloc(sizeof(int) * S->pro_count);//�غ�ť�� �ְų� �־��� ���μ����� ���̵� ������ �迭
	init_queue(&q, S);//ť �ʱ�ȭ

	while (S->pro_count != end_pro) {//�����ٷ��� �ִ� ���μ����� ������ ����� ���μ����� ������ ���� ���� ������ �ݺ�
		if (check_count != S->pro_count) {//ť�� ���� ���� ���� ���μ����� �����ϴ� ���
			for (int i = 0; i < S->pro_count; i++) {//�����ٷ��� �ִ� ���μ����� ������ŭ �ݺ�
				if (S->list[i].arr_t <= cur_time) {//�����ٷ��� �ִ� ���μ��� �� ���õ� ���μ����� �����ð��� ���� �ð����� �۰ų� ���� ���
					for (int j = 0; j < S->pro_count; j++)//�̹� ť�� ���Դ� ���μ������� �˻��ϱ� ���� �ݺ���
						if (S->list[i].pro_id == check[j])dup = 1;//id�� ���� ���μ����� �ִٸ� dup�� 1�� ����
					if (dup == 0) {//dup�� 0�� ���� ���� ���μ����� ť�� ���� �� ���� ���μ����� ���
						enqueue(&q, S->list[i]);//ť�� �ش� ���μ��� ����
						check_count++;
						check[S->list[i].index] = S->list[i].pro_id; //���� ���μ����� ���̵� ����
					}
					else//���� ���μ����� ť�� ���Ծ��� ���μ����� ���
						dup = 0;
				}
			}
		}
		if (cur_time == 0) temp = dequeue(&q);//���� �ð��� 0�� �� ù��° ���μ����� ��ť

		if (time_slice == S->time_slice && temp.ser_t != 0) {//���� ���μ����� �־��� Ÿ�ӽ����̽��� �� ������ �۾��� ������ ���� ����
			printf("[%d] | ", ganttChart);//��Ʈ��Ʈ�� ��µ� ���� �ð� ���
			ganttChart = 0;//�������Ƿ� 0���� �ʱ�ȭ
			time_slice = 0;//���� ���μ����� �ð��� �����ϱ� ���� 0���� �ʱ�ȭ
			enqueue(&q, temp);//�۾��� �������Ƿ� �ٽ� ť�� ����
			temp = dequeue(&q);//��ť
		}
		else if (temp.ser_t == 0) {//���� ���μ����� ����� ���·� �����ٷ��� �ش� ���μ����� ������ ����ؼ� ����
			printf("[%d] | ", ganttChart);//��Ʈ��Ʈ�� ��µ� ���� �ð� ���
			ganttChart = 0;//�������Ƿ� 0���� �ʱ�ȭ
			end_pro++;
			time_slice = 0; //���� ���μ����� �ð��� �����ϱ� ���� 0���� �ʱ�ȭ
			S->list[temp.index].wait_t = cur_time - S->list[temp.index].ser_t - temp.arr_t;
			S->list[temp.index].turn_a_t = S->list[temp.index].wait_t + S->list[temp.index].ser_t;
			wt_sum += S->list[temp.index].wait_t;
			tt_sum += S->list[temp.index].turn_a_t;
			rt_sum += S->list[temp.index].res_t;
			if (!is_empty(&q))//ť�� ������ �ƴ� ���
				temp = dequeue(&q);//��ť
		}
		if (S->pro_count != end_pro) {//��� ���μ����� ����� ��쿡�� ��µǴ� ���� �����ϱ� ���� ���ǹ�
			printf("P%d", temp.pro_id);//���� ���μ����� ��Ʈ��Ʈ ���
			ganttChart++;//��Ʈ��Ʈ�� ��µ� ���� �ð� ����
		}
		temp.count++;//�ش� ���μ����� ���� ����ð� ����
		cur_time++;//���� �ð� ����
		if (temp.count == RT)S->list[temp.index].res_t = cur_time - temp.arr_t;//�ش� ���μ����� ���� ����ð��� �־��� �����ð��� �����Ƿ� �����ð� �Է�
		time_slice++;//�ش� ���μ����� ���ð� ����
		temp.ser_t--;//�ش� ���μ����� ���� �ð� ����

	}
	S->awt = (double)wt_sum / S->pro_count;
	S->att = (double)tt_sum / S->pro_count;
	S->art = (double)rt_sum / S->pro_count;
	print_scheduler(S);//�����ٷ��� ���� ���
	free(q.list);//�޸� ��ȯ
	free(check);//�޸� ��ȯ
}

//SRT �����ٸ� �Լ�
void SRT_Scheduling(scheduler* S) {
	printf("<SRT�����ٸ�>\n[��Ʈ��Ʈ]\n");
	reset(S);//�����ٷ� ���� �ʱ�ȭ(ID, �����ð�, ���� �ð�, �켱����, ���μ��� ����, Ÿ�ӽ����̽�, �ε��� ����)
	arr_t_sort(S);//���� ������ ����
	queuetype q;//ť ����
	process temp;//ť���� ���� ���μ����� ������ ������ ����
	int  wt_sum = 0, tt_sum = 0, rt_sum = 0, cur_time = 0, end_pro = 0, dup = 0, check_count = 0, time_slice = 0, ganttChart = 0;//����κ� �Լ��� ������ ���� ����
	int* check = (int*)malloc(sizeof(int) * S->pro_count);//�غ�ť�� �ְų� �־��� ���μ����� ���̵� ������ �迭
	init_queue(&q, S);//ť �ʱ�ȭ

	while (S->pro_count != end_pro) {//�����ٷ��� �ִ� ���μ����� ������ ����� ���μ����� ������ ���� ���� ������ �ݺ�
		if (check_count != S->pro_count) {//ť�� ���� ���� ���� ���μ����� �����ϴ� ���
			for (int i = 0; i < S->pro_count; i++) {//�����ٷ��� �ִ� ���μ����� ������ŭ �ݺ�
				if (S->list[i].arr_t <= cur_time) {//�����ٷ��� �ִ� ���μ��� �� ���õ� ���μ����� �����ð��� ���� �ð����� �۰ų� ���� ���
					for (int j = 0; j < S->pro_count; j++)//�̹� ť�� ���Դ� ���μ������� �˻��ϱ� ���� �ݺ���
						if (S->list[i].pro_id == check[j])dup = 1;//id�� ���� ���μ����� �ִٸ� dup�� 1�� ����
					if (dup == 0) {//dup�� 0�� ���� ���� ���μ����� ť�� ���� �� ���� ���μ����� ���
						enqueue(&q, S->list[i]);//ť�� �ش� ���μ��� ����
						if (q.count > 1)//ť�� 2�� �̻��� ���μ����� �ִ� ��� ���� �ð� �������� ����
							queue_ser_t_sort(&q);
						check_count++;
						check[S->list[i].index] = S->list[i].pro_id;//���� ���μ����� ���̵� ����
					}
					else//���� ���μ����� ť�� ���Ծ��� ���μ����� ���
						dup = 0;
				}
			}
		}
		if (cur_time == 0)//���� �ð��� 0�� �� ù��° ���μ����� ��ť
			temp = dequeue(&q);

		if (time_slice == S->time_slice && temp.ser_t != 0) {//���� ���μ����� �־��� Ÿ�ӽ����̽��� �� ������ �۾��� ������ ���� ����
			printf("[%d] | ", ganttChart);//��Ʈ��Ʈ�� ��µ� ���� �ð� ���
			ganttChart = 0;//�������Ƿ� 0���� �ʱ�ȭ
			time_slice = 0;//���� ���μ����� �ð��� �����ϱ� ���� 0���� �ʱ�ȭ
			enqueue(&q, temp);//�۾��� �������Ƿ� �ٽ� ť�� ����
			if (q.count > 1)//ť�� 2�� �̻��� ���μ����� �ִ� ��� ���� �ð� �������� ����
				queue_ser_t_sort(&q);
			temp = dequeue(&q);
		}
		else if (temp.ser_t == 0) {//���� ���μ����� ����� ���·� �����ٷ��� �ش� ���μ����� ������ ����ؼ� ����
			printf("[%d] | ", ganttChart);//��Ʈ��Ʈ�� ��µ� ���� �ð� ���
			ganttChart = 0;//�������Ƿ� 0���� �ʱ�ȭ
			end_pro++;
			time_slice = 0;//���� ���μ����� �ð��� �����ϱ� ���� 0���� �ʱ�ȭ
			S->list[temp.index].wait_t = cur_time - S->list[temp.index].ser_t - temp.arr_t;
			S->list[temp.index].turn_a_t = S->list[temp.index].wait_t + S->list[temp.index].ser_t;
			wt_sum += S->list[temp.index].wait_t;
			tt_sum += S->list[temp.index].turn_a_t;
			rt_sum += S->list[temp.index].res_t;
			if (!is_empty(&q))//ť�� ������ �ƴ� ��� ��ť
				temp = dequeue(&q);
		}
		if (S->pro_count != end_pro) {//��� ���μ����� ����� ��쿡�� ��µǴ� ���� �����ϱ� ���� ���ǹ�
			printf("P%d", temp.pro_id);//���� ���μ����� ��Ʈ��Ʈ ���
			ganttChart++;//��Ʈ��Ʈ�� ��µ� ���� �ð� ����
		}
		temp.count++;//�ش� ���μ����� ���� ����ð� ����
		cur_time++;//���� �ð� ����
		if (temp.count == RT)S->list[temp.index].res_t = cur_time - temp.arr_t;//�ش� ���μ����� ���� ����ð��� �־��� �����ð��� �����Ƿ� �����ð� �Է�
		time_slice++;//�ش� ���μ����� ���ð� ����
		temp.ser_t--;//�ش� ���μ����� ���� �ð� ����
	}
	S->awt = (double)wt_sum / S->pro_count;
	S->att = (double)tt_sum / S->pro_count;
	S->art = (double)rt_sum / S->pro_count;
	print_scheduler(S);//�����ٷ��� ���� ���
	free(q.list);//�޸� ��ȯ
	free(check);//�޸� ��ȯ
}

//������ �켱���� �����ٸ� �Լ�
void Priority_Scheduling(scheduler* S) {
	printf("<������ �켱���� �����ٸ�>\n[��Ʈ��Ʈ]\n");
	reset(S);//�����ٷ� ���� �ʱ�ȭ(ID, �����ð�, ���� �ð�, �켱����, ���μ��� ����, Ÿ�ӽ����̽�, �ε��� ����)
	arr_t_sort(S);//���� ������ ����
	queuetype q;//ť ����
	process temp;//ť���� ���� ���μ����� ������ ������ ����
	int  wt_sum = 0, tt_sum = 0, rt_sum = 0, cur_time = 0, end_pro = 0, dup = 0, check_count = 0, ganttChart = 0;//����κ��� ������� ���� ����
	int* check = (int*)malloc(sizeof(int) * S->pro_count);//�غ�ť�� �ְų� �־��� ���μ����� ���̵� ������ �迭
	init_queue(&q, S);//ť �ʱ�ȭ

	while (S->pro_count != end_pro) {//�����ٷ��� �ִ� ���μ����� ������ ����� ���μ����� ������ ���� ���� ������ �ݺ�
		if (check_count != S->pro_count) {//ť�� ���� ���� ���� ���μ����� �����ϴ� ���
			for (int i = 0; i < S->pro_count; i++) {//�����ٷ��� �ִ� ���μ����� ������ŭ �ݺ�
				if (S->list[i].arr_t <= cur_time) {//�����ٷ��� �ִ� ���μ��� �� ���õ� ���μ����� �����ð��� ���� �ð����� �۰ų� ���� ���
					for (int j = 0; j < S->pro_count; j++)//�̹� ť�� ���Դ� ���μ������� �˻��ϱ� ���� �ݺ���
						if (S->list[i].pro_id == check[j])dup = 1;//id�� ���� ���μ����� �ִٸ� dup�� 1�� ����
					if (dup == 0) {//dup�� 0�� ���� ���� ���μ����� ť�� ���� �� ���� ���μ����� ���
						enqueue(&q, S->list[i]);//ť�� �ش� ���μ��� ����
						if (q.count > 1)//ť�� ���μ����� 2�� �̻��� ��� �켱���� �������� ����
							queue_pro_pri_sort(&q);
						check_count++;
						check[S->list[i].index] = S->list[i].pro_id;//���� ���μ����� ���̵� ����
					}
					else//���� ���μ����� ť�� ���Ծ��� ���μ����� ���
						dup = 0;
				}
			}
		}
		if (cur_time == 0)//���� �ð��� 0�� �� ù��° ���μ����� ��ť
			temp = dequeue(&q);

		if (q.count != 0 && temp.ser_t != 0 && temp.pro_pri > q.list[q.front + 1].pro_pri) {//���� ���μ����� �۾� �ð��� ���������� ť�� ���μ����� 1�� �̻� �ְ� ���� �տ� �ִ� ���μ����� �켱 ������ ���� ���μ����� �켱�������� ���� ���
			printf("[%d] | ", ganttChart);//��Ʈ��Ʈ�� ��µ� ���� �ð� ���
			ganttChart = 0;//�������Ƿ� 0���� �ʱ�ȭ
			enqueue(&q, temp);//�۾��� �������Ƿ� �ٽ� ť�� ����
			if (q.count > 1)//ť�� ���μ����� 2�� �̻� �ִ� ���
				queue_pro_pri_sort(&q);//�켱���� �������� ����
			temp = dequeue(&q);//��ť
		}
		else if (temp.ser_t == 0) {//���� ���μ����� ����� ���·� �����ٷ��� �ش� ���μ����� ������ ����ؼ� ����
			printf("[%d] | ", ganttChart);//��Ʈ��Ʈ�� ��µ� ���� �ð� ���
			ganttChart = 0;//�������Ƿ� 0���� �ʱ�ȭ
			end_pro++;
			S->list[temp.index].wait_t = cur_time - S->list[temp.index].ser_t - temp.arr_t;
			S->list[temp.index].turn_a_t = S->list[temp.index].wait_t + S->list[temp.index].ser_t;
			wt_sum += S->list[temp.index].wait_t;
			tt_sum += S->list[temp.index].turn_a_t;
			rt_sum += S->list[temp.index].res_t;
			if (!is_empty(&q))//ť�� ������ �ƴ� ��� ��ť
				temp = dequeue(&q);
		}
		if (S->pro_count != end_pro) {//��� ���μ����� ����� ��쿡�� ��µǴ� ���� �����ϱ� ���� ���ǹ�
			printf("P%d", temp.pro_id);//���� ���μ����� ��Ʈ��Ʈ ���
			ganttChart++;//��Ʈ��Ʈ�� ��µ� ���� �ð� ����
		}

		temp.count++;//�ش� ���μ����� ���� ����ð� ����
		cur_time++;//���� �ð� ����
		if (temp.count == RT)S->list[temp.index].res_t = cur_time - temp.arr_t;//�ش� ���μ����� ���� ����ð��� �־��� �����ð��� �����Ƿ� �����ð� �Է�
		temp.ser_t--;//�ش� ���μ����� ���� �ð� ����
	}
	S->awt = (double)wt_sum / S->pro_count;
	S->att = (double)tt_sum / S->pro_count;
	S->art = (double)rt_sum / S->pro_count;
	print_scheduler(S);//�����ٷ��� ���� ���
	free(q.list);//�޸� ��ȯ
	free(check);//�޸� ��ȯ
}

//���α׷� ���� �Լ�
void run(scheduler* S) {
	int menu;//����ڰ� �Է��� �޴��� ������ ����
	print_process_file(S);//���Ͽ��� �о�� ���μ����� ������ ���
	while (1) {//���� �ݺ���
		print_menu();//�޴� ���
		printf("�޴��� �������ּ���: ");
		scanf("%d", &menu);
		getchar();//���鹮�� ����
		system("cls");//�ܼ� ȭ�� �����
		switch (menu)
		{
		case 1://1�� ��� FCFS �����ٸ� ȣ��
			FCFS_Scheduling(S);
			break;
		case 2://2�� ��� SJF �����ٸ� ȣ��
			SJF_Scheduling(S);
			break;
		case 3://3�� ��� HRN �����ٸ� ȣ��
			HRN_Scheduling(S);
			break;
		case 4://4�� ��� ������ �켱���� �����ٸ� ȣ��
			Non_Priority_Scheduling(S);
			break;
		case 5://5�� ��� ����κ� �����ٸ� ȣ��
			RR_Scheduling(S);
			break;
		case 6://6�� ��� SRT �����ٸ� ȣ��
			SRT_Scheduling(S);
			break;
		case 7://7�� ��� ������ �켱���� �����ٸ� ȣ��
			Priority_Scheduling(S);
			break;
		case 8://�޸� ���� �� ����
			free(S->list);//�޸� ����
			exit(1);
		default:
			print_process_file(S);//�ٸ� ���� �Է½� ���Ͽ��� �о�� ���μ����� ���� �����
		}
	}
}

int main() {
	scheduler S;//�����ٷ� ����
	int index = 0;//���μ����� �����ٷ��� �迭�� ����� �ε����� 
	char a;

	FILE* fp = fopen("data.txt", "r");//������ �б� ���� ����
	if (fp == NULL) {//���� �����Ͱ� NULL�� ��� ���� ���� ���� ���� ��� �� ����
		printf("���� ����\n");
		return 0;
	}

	fscanf(fp, "%d", &S.pro_count);//������ ù ��° ���ڸ� ������ ����
	S.list = (process*)malloc(sizeof(process) * S.pro_count);//���μ����� ����ŭ list�迭�� �޸� ���� �Ҵ�
	if (S.list == NULL) {//list�� NULL�� ��� �޸� �Ҵ� ����
		printf("���� �Ҵ� ����\n");
		return 0;
	}
	for (int i = 0; i < S.pro_count; i++) {//���μ��� ���� ��ŭ �ݺ��Ͽ� ���Ͽ� �ִ� ���μ��� ���� ����
		fscanf(fp, "%c", &a);
		fscanf(fp, "%c", &a);
		fscanf(fp, "%d", &S.list[i].pro_id);
		fscanf(fp, "%d", &S.list[i].arr_t);
		fscanf(fp, "%d", &S.list[i].ser_t);
		fscanf(fp, "%d", &S.list[i].pro_pri);
		S.list[i].index = i;
	}
	fscanf(fp, "%d", &S.time_slice);//Ÿ�� �����̽��� �о�� ����
	fclose(fp);//���� �ݱ�
	run(&S);
	return 0;
}