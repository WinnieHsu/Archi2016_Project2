#ifndef SINGLE_CYCLE_H_INCLUDED
#define SINGLE_CYCLE_H_INCLUDED

void initialize();
void read_file();
void store_dmemory(char ch);
void store_imemory(char ch);
void initial_SNAP();
void PC_adder();
void append_SNAP();

void instruction_fetch();
void instruction_decode();
void execution(int index);
void memory(int index);
void write_back(int index);


/**R-type instructions**/
void add(int rs, int rt, int rd);
void addu(int rs, int rt, int rd);
void sub(int, int, int);
void and(int, int, int);
void or(int, int, int);
void xor(int, int, int);
void nor(int, int, int);
void nand(int, int, int);
void slt(int, int, int);
void sll(int, int, int);
void srl(int, int, int);
void sra(int, int, int);
void jr(int);
/**J-type instructions**/
void jj(int);
void jal(int);
/**I-type instructions**/
void addi(int, int, int);
void addiu(int, int, int);
void lw(int, int, int);
void lh(int, int, int);
void lhu(int, int, int);
void lb(int, int, int);
void lbu(int, int, int);
void sw(int, int, int);
void sh(int, int, int);
void sb(int, int, int);
void lui(int, int);
void andi(int, int, int);
void ori(int, int, int);
void nori(int, int, int);
void slti(int, int, int);
void beq(int, int, int);
void bne(int, int, int);
void bgtz(int, int);

//int HEXtoDEC_bit(char c);
char DECtoHEX_bit(int n);
//int char_HEXtoDEC(char arr[], int n_bits, int start);
//int int_HEXtoDEC(int arr[], int n_bits, int start);
int char_BINtoDEC(char arr[], int n_bits, int start);
int signed_char_BINtoDEC(char arr[], int n_bits, int start);
int int_BINtoDEC(int arr[], int n_bits, int start);
int signed_int_BINtoDEC(int arr[], int n_bits, int start);
int flip(int n);

#endif // SINGLE_CYCLE_H_INCLUDED
