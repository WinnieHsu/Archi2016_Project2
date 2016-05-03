#include <stdio.h>
#include "pipeline.h"

int im_index=0, dm_index=0;
char im_input[1026][10]={}, dm_input[1026][10]={};
char REG[32][34]={}; //denote the value of register $zero, at, v0~v1, a0~a3, t0~t7, s0~s7, t7~t8, k0~k1, gp, sp, fp, ra
char imemory[256][34]={}, dmemory[256][34]={};
int INSTR_num=0, MEM_num=0;
char PC_init[34]={}, PC_now[34]={};
int PC_initptr=0, PC_nowptr=0;

int OP, RS, RT, RD, FUNCT, SHAMT, C, signedC;
int ID_index=0, EX_index=0, DM_index=0, WB_index=0;

int cycle=0, halt=0;
int stall=0, forward=0;
int write$0_error=0, number_overflow=0, memory_overflow=0, misaligned=0;

FILE *snap, *error;

int test=1; //whether to printf

void initial_SNAP(){
    int i, j;
    char SP_hex[10]={}, PC_hex[10]={};
    fprintf(snap,"cycle 0\n");
    for(i=0; i<32; i++){
        if(i<10) fprintf(snap,"$0%d: 0x",i);
        else fprintf(snap,"$%d: 0x",i);

        if(i==29){
            //convert SP from binary to hexadecimal
            for(j=7; j>=0; j--){
                SP_hex[j] = DECtoHEX_bit( char_BINtoDEC(REG[29],4,(j+1)*4-1) );
            }
            for(j=0; j<8; j++){
                fprintf(snap,"%c",SP_hex[j]);
            }
        }
        else fprintf(snap,"00000000");
        fprintf(snap,"\n");
    }

    fprintf(snap,"PC: 0x");
    //convert PC from binary to hexadecimal
    for(j=7; j>=0; j--)
        PC_hex[j] = DECtoHEX_bit( char_BINtoDEC(PC_now,4,(j+1)*4-1) );
    for(j=0; j<8; j++)
        fprintf(snap,"%c",PC_hex[j]);

    fprintf(snap,"\nIF: 0x");
    for(j=0; j<8; j++)
        fprintf(snap,"%c",PC_hex[j]);
    fprintf(snap,"\nID: NOP\n");
    fprintf(snap,"EX: NOP\n");
    fprintf(snap,"DM: NOP\n");
    fprintf(snap,"WB: NOP\n");

    fprintf(snap,"\n\n\n");
    cycle++;
}
void PC_adder(){
    int carry=0, bitsum, j=29;
    int add[34]={0}; add[29]=1;

    //for(i=0; i<5; i++){
        //carry=0;
        //j=29;
        bitsum=(PC_now[j]-'0')+add[j]+carry;
        if(bitsum<2){
            PC_now[j]=bitsum+'0';
            carry=0;
        }
        else{
            PC_now[j]=bitsum-2+'0';
            carry=1;
            j--;
            while(carry && j>=0){
                bitsum=(PC_now[j]-'0')+add[j]+carry;
                if(bitsum<2){
                    PC_now[j]=bitsum+'0';
                    carry=0;
                    break;
                }
                else{
                    PC_now[j]=bitsum-2+'0';
                    carry=1;
                    j--;
                }
            }
        }
        PC_nowptr=char_BINtoDEC(PC_now,32,31);
    //}
}
void append_SNAP(){

    snap=fopen("snapshot.rpt","a");

    int i, j;
    char REG_hex[10]={}, PC_hex[10]={};

    fprintf(snap,"cycle %d\n",cycle);
    for(i=0; i<32; i++){
        if(i<10) fprintf(snap,"$0%d: 0x",i);
        else fprintf(snap,"$%d: 0x",i);

        for(j=7; j>=0; j--)
            REG_hex[j] = DECtoHEX_bit( char_BINtoDEC(REG[i],4,(j+1)*4-1) );
        for(j=0; j<8; j++)
            fprintf(snap,"%c",REG_hex[j]);

        fprintf(snap,"\n");
    }

    fprintf(snap,"PC: 0x");
    //convert PC from binary to hexadecimal
    for(j=7; j>=0; j--)
        PC_hex[j] = DECtoHEX_bit( char_BINtoDEC(PC_now,4,(j+1)*4-1) );
    for(j=0; j<8; j++)
        fprintf(snap,"%c",PC_hex[j]);

    fprintf(snap,"IF: 0x");
    fprintf(snap,"ID: "); display_instruction(ID_index);
    fprintf(snap,"EX: "); display_instruction(EX_index);
    fprintf(snap,"DM: "); display_instruction(DM_index);
    fprintf(snap,"WB: "); display_instruction(WB_index);

    fprintf(snap,"\n\n\n");
    cycle++;
}

void display_instruction(int index){
    if(index==0){
        printf("NOP");
    }
    /**R-type instructions**/
    else if(index==1){
        printf("ADD");
    }
    else if(index==2){
        printf("ADDU");
    }
    else if(index==3){
        printf("SUB");
    }
    else if(index==4){
        printf("AND");
    }
    else if(index==5){
        printf("OR");
    }
    else if(index==6){
        printf("XOR");
    }
    else if(index==7){
        printf("NOR");
    }
    else if(index==8){
        printf("NAND");
    }
    else if(index==9){
        printf("SLT);
    }
    else if(index==10){
        printf("SLL");
    }
    else if(index==11){
        printf("SRL");
    }
    else if(index=12){
        printf("SRA");
    }
    /**J-type instructions**/
    else if(index==13){
        printf("JR");
    }
    else if(index==14){
        printf("J");
    }
    else if(index==15){
        printf("JAL");
    }
    /**I-type instructions**/
    else if(index==16){
        printf("ADDI");
    }
    else if(index==17){
        printf("ADDIU");
    }
    else if(index==18){
        printf("LW");
    }
    else if(index==19){
        printf("LH");
    }
    else if(index==20){
        printf("LHU");
    }
    else if(index==21){
        printf("LB");
    }
    else if(index==22){
        printf("LBU");
    }
    else if(index==23){
        printf("SW");
    }
    else if(index==24){
        printf("SH");
    }
    else if(index==25){
        printf("SB");
    }
    else if(index==26){
        printf("LUI");
    }
    else if(index==27){
        printf("ANDI");
    }
    else if(index==28){
        printf("OR");
    }
    else if(index==29){
        printf("NOR");
    }
    else if(index==30){
        printf("SLTI");
    }
    else if(index==31){
        printf("BEQ");
    }
    else if(index==32){
        printf("BNE");
    }
    else if(index==33){
        printf("BGTZ");
    }
    else{
        if(test==1) printf("this is error instruction\n");
    }
}

void store_imemory(char ch){
    int i;
    unsigned char mask=0x80;
    for (i=0; i<8; i++){
        if(ch&mask) im_input[im_index][i]='1';
        else im_input[im_index][i]='0';
        mask=mask>>1;
    }
    im_index++;
}
void store_dmemory(char ch){
    int i;
    unsigned char mask=0x80;
    for (i=0; i<8; i++){
        if(ch&mask) dm_input[dm_index][i]='1';
        else dm_input[dm_index][i]='0';
        mask=mask>>1;
    }
    dm_index++;
}
void read_file(){
    char ch;
    FILE *fin;

    fin=fopen("iimage.bin","rt");

    while(!feof(fin)){
        fscanf(fin,"%c",&ch);
        store_imemory(ch);
    }
    fin=fopen("dimage.bin","rt");
    while(!feof(fin)){
        fscanf(fin,"%c",&ch);
        store_dmemory(ch);
    }
    fclose(fin);
}

void initialize(){
    int i,j;
    //initialize REG, imemory, dmemory
    for(i=0; i<32; i++){
        for(j=0; j<34; j++){
            REG[i][j]='0';
        }
    }
    for(i=0; i<256; i++){
        for(j=0; j<34; j++){
            dmemory[i][j]='0';
            imemory[i][j]='0';
        }
    }

    read_file();
    //store the value of PC, SP
    for(i=0; i<8; i++){
        PC_init[i]=im_input[0][i];
        PC_init[i+8]=im_input[1][i];
        PC_init[i+16]=im_input[2][i];
        PC_init[i+24]=im_input[3][i];

        PC_now[i]=im_input[0][i];
        PC_now[i+8]=im_input[1][i];
        PC_now[i+16]=im_input[2][i];
        PC_now[i+24]=im_input[3][i];
    }
    PC_initptr=char_BINtoDEC(PC_init, 32, 31);
    PC_nowptr=PC_initptr;

    if(test==1) printf("enter initialize with PC_initptr=%d\n",PC_initptr);
    for(i=0; i<8; i++){ //SP
        REG[29][i]=dm_input[0][i];
        REG[29][i+8]=dm_input[1][i];
        REG[29][i+16]=dm_input[2][i];
        REG[29][i+24]=dm_input[3][i];
    }

    //store the number of MEM, INSTR
    int NUM[34]={0};
    for(i=0; i<8; i++){
        NUM[i]=dm_input[4][i]-'0';
        NUM[i+8]=dm_input[5][i]-'0';
        NUM[i+16]=dm_input[6][i]-'0';
        NUM[i+24]=dm_input[7][i]-'0';
    }
    MEM_num=int_BINtoDEC(NUM,32,31);

    for(i=0; i<8; i++){
        NUM[i]=im_input[4][i]-'0';
        NUM[i+8]=im_input[5][i]-'0';
        NUM[i+16]=im_input[6][i]-'0';
        NUM[i+24]=im_input[7][i]-'0';
    }
    INSTR_num=int_BINtoDEC(NUM,32,31);
    if(test==1) printf("enter initialize with MEM_num=%d, INSTR_num=%d\n",MEM_num,INSTR_num);

    //store the content of memory into dmemory
    for(i=0; i<MEM_num; i++){
        for(j=0; j<8; j++){
            dmemory[i][j]=dm_input[8+4*i][j];
            dmemory[i][j+8]=dm_input[9+4*i][j];
            dmemory[i][j+16]=dm_input[10+4*i][j];
            dmemory[i][j+24]=dm_input[11+4*i][j];
        }
    }

    //store the content of instructions into imemory
    for(i=0; i<INSTR_num; i++){
        for(j=0; j<8; j++){
            imemory[PC_initptr/4+i][j]=im_input[8+4*i][j];
            imemory[PC_initptr/4+i][j+8]=im_input[9+4*i][j];
            imemory[PC_initptr/4+i][j+16]=im_input[10+4*i][j];
            imemory[PC_initptr/4+i][j+24]=im_input[11+4*i][j];
        }
    }
    if(test==1) printf("finish initialize register and memory\n");

}

void instruction_fetch(){

    if(test==1) printf("enter IF with PC_nowptr=%d\n",PC_nowptr);
    PC_adder(); //PC_now+4
    if(test==1) printf("enter IF after PC_adder() PC_nowptr=%d\n",PC_nowptr);
    //fetch the instruction of imemory[PC_nowptr/4-1]

    OP = char_BINtoDEC(imemory[PC_nowptr/4-1],6,5);
    RS = char_BINtoDEC(imemory[PC_nowptr/4-1],5,10);
    RT = char_BINtoDEC(imemory[PC_nowptr/4-1],5,15);

}
void instruction_decode(){

    /**decode each instruction**/
    int i, j, INSTR_index;

        /**analyze INSTR to OPcode, RS, RT, RD, etc.**/
        //OP = char_BINtoDEC(imemory[PC_nowptr/4-1],6,5);
        if(OP==0){
            FUNCT = char_BINtoDEC(imemory[PC_nowptr/4-1],6,31);
            //RS = char_BINtoDEC(imemory[PC_nowptr/4-1],5,10);
            //RT = char_BINtoDEC(imemory[PC_nowptr/4-1],5,15);
            RD = char_BINtoDEC(imemory[PC_nowptr/4-1],5,20);
            SHAMT = char_BINtoDEC(imemory[PC_nowptr/4-1],5,25);

            if(FUNCT==32){
                INSTR_index=1;
                //add(RS,RT,RD);
                if(test==1) printf("this is R-type add(%d,%d,%d)\n",RS,RT,RD);
            }
            else if(FUNCT==33){
                INSTR_index=2;
                //addu(RS,RT,RD);
                if(test==1) printf("this is R-type addu(%d,%d,%d)\n",RS,RT,RD);
            }
            else if(FUNCT==34){
                INSTR_index=3;
                //sub(RS,RT,RD);
                if(test==1) printf("this is R-type sub(%d,%d,%d)\n",RS,RT,RD);
            }
            else if(FUNCT==36){
                INSTR_index=4;
                //and(RS,RT,RD);
                if(test==1) printf("this is R-type and(%d,%d,%d)\n",RS,RT,RD);
            }
            else if(FUNCT==37){
                INSTR_index=5;
                //or(RS,RT,RD);
                if(test==1) printf("this is R-type or(%d,%d,%d)\n",RS,RT,RD);
            }
            else if(FUNCT==38){
                INSTR_index=6;
                //xor(RS,RT,RD);
                if(test==1) printf("this is R-type xor(%d,%d,%d)\n",RS,RT,RD);
            }
            else if(FUNCT==39){
                INSTR_index=7;
                //nor(RS,RT,RD);
                if(test==1) printf("this is R-type nor(%d,%d,%d)\n",RS,RT,RD);
            }
            else if(FUNCT==40){
                INSTR_index=8;
                //nand(RS,RT,RD);
                if(test==1) printf("this is R-type nand(%d,%d,%d)\n",RS,RT,RD);
            }
            else if(FUNCT==42){
                INSTR_index=9;
                //slt(RS,RT,RD);
                if(test==1) printf("this is R-type slt(%d,%d,%d)\n",RS,RT,RD);
            }
            else if(FUNCT==0){
                INSTR_index=10;
                //sll(RT,RD,SHAMT);
                if(test==1) printf("this is R-type sll(%d,%d,%d)\n",RT,RD,SHAMT);
            }
            else if(FUNCT==2){
                INSTR_index=11;
                //srl(RT,RD,SHAMT);
                if(test==1) printf("this is R-type srl(%d,%d,%d)\n",RT,RD,SHAMT);
            }
            else if(FUNCT==3){
                INSTR_index=12;
                //sra(RT,RD,SHAMT);
                if(test==1) printf("this is R-type sra(%d,%d,%d)\n",RT,RD,SHAMT);
            }
            else if(FUNCT==8){
                INSTR_index=13;
                stall=1;
                //jr(RS);
                if(test==1) printf("this is R-type jr(%d)\n",RS);
            }
            else{
                if(test==1) printf("this is error R-instruction\n");
            }
        }
        else if(OP==2||OP==3||OP==63){
            C = char_BINtoDEC(imemory[PC_nowptr/4-1],26,31);

            if(OP==2){
                INSTR_index=14;
                //jj(C);
                if(test==1) printf("this J-type jj(%d)\n",C);
            }
            else if(OP==3){
                INSTR_index=15;
                //jal(C);
                if(test==1) printf("this J-type jal(%d)\n",C);
            }
            else if(OP==63){
                INSTR_index=0;
                halt=1;
            }
            else{
                if(test==1) printf("this is error J-instruction\n");
            }
        }
        else{
            //RS = char_BINtoDEC(imemory[PC_nowptr/4-1],5,10);
            //RT = char_BINtoDEC(imemory[PC_nowptr/4-1],5,15);
            C = char_BINtoDEC(imemory[PC_nowptr/4-1],16,31);
            signedC = signed_char_BINtoDEC(imemory[PC_nowptr/4-1],16,31);

            if(OP==8){
                INSTR_index=16;
                //addi(RS,RT,C);
                if(test==1) printf("this I-type addi(%d,%d,%d)\n",RS,RT,signedC);
            }
            else if(OP==9){
                INSTR_index=17;
                //addiu(RS,RT,C);
                if(test==1) printf("this I-type addiu(%d,%d,%d)\n",RS,RT,C);
            }
            else if(OP==35){
                INSTR_index=18;
                //lw(RS,RT,signedC);
                if(test==1) printf("this I-type lw(%d,%d,%d)\n",RS,RT,signedC);
            }
            else if(OP==33){
                INSTR_index=19;
                //lh(RS,RT,signedC);
                if(test==1) printf("this I-type lh(%d,%d,%d)\n",RS,RT,signedC);
            }
            else if(OP==37){
                INSTR_index=20;
                //lhu(RS,RT,C);
                if(test==1) printf("this I-type lhu(%d,%d,%d)\n",RS,RT,C);
            }
            else if(OP==32){
                INSTR_index=21;
                //lb(RS,RT,signedC);
                if(test==1) printf("this I-type lb(%d,%d,%d)\n",RS,RT,signedC);
            }
            else if(OP==36){
                INSTR_index=22;
                //lbu(RS,RT,C);
                if(test==1) printf("this I-type lbu(%d,%d,%d)\n",RS,RT,C);
            }
            else if(OP==43){
                INSTR_index=23;
                //sw(RS,RT,signedC);
                if(test==1) printf("this I-type sw(%d,%d,%d)\n",RS,RT,signedC);
            }
            else if(OP==41){
                INSTR_index=24;
                //sh(RS,RT,signedC);
                if(test==1) printf("this I-type sh(%d,%d,%d)\n",RS,RT,signedC);
            }
            else if(OP==40){
                INSTR_index=25;
                //sb(RS,RT,signedC);
                if(test==1) printf("this I-type sb(%d,%d,%d)\n",RS,RT,signedC);
            }
            else if(OP==15){
                INSTR_index=26;
                //lui(RT,C);
                if(test==1) printf("this I-type lui(%d,%d)\n",RT,C);
            }
            else if(OP==12){
                INSTR_index=27;
                //andi(RS,RT,C);
                if(test==1) printf("this I-type andi(%d,%d,%d)\n",RS,RT,C);
            }
            else if(OP==13){
                INSTR_index=28;
                //ori(RS,RT,C);
                if(test==1) printf("this I-type ori(%d,%d,%d)\n",RS,RT,C);
            }
            else if(OP==14){
                INSTR_index=29;
                //nori(RS,RT,C);
                if(test==1) printf("this I-type nori(%d,%d,%d)\n",RS,RT,C);
            }
            else if(OP==10){
                INSTR_index=30;
                //slti(RS,RT,C);
                if(test==1) printf("this I-type slti(%d,%d,%d)\n",RS,RT,C);
            }
            else if(OP==4){
                INSTR_index=31;
                stall=1;
                //beq(RS,RT,signedC);
                if(test==1) printf("this I-type beq(%d,%d,%d)\n",RS,RT,signedC);
            }
            else if(OP==5){
                INSTR_index=32;
                stall=1;
                //bne(RS,RT,signedC);
                if(test==1) printf("this I-type bne(%d,%d,%d)\n",RS,RT,signedC);
            }
            else if(OP==7){
                INSTR_index=33;
                stall=1;
                //bgtz(RS,signedC);
                if(test==1) printf("this I-type bgtz(%d,%d)\n",RS,signedC);
            }
            else{
                if(test==1) printf("this is I-error instruction\n");
            }
        }
        ID_index=INSTR_index;
        //execution(INSTR_index);
        //if(PC_nowptr>PC_initptr+INSTR_num) break;
}

void execution(int index){
    if(index==0){ //NOP
        return;
    }
    /**R-type instructions**/
    else if(index==1){
        add(RS,RT,RD);
    }
    else if(index==2){
        addu(RS,RT,RD);
    }
    else if(index==3){
        sub(RS,RT,RD);
    }
    else if(index==4){
        and(RS,RT,RD);
    }
    else if(index==5){
        or(RS,RT,RD);
    }
    else if(index==6){
        xor(RS,RT,RD);
    }
    else if(index==7){
        nor(RS,RT,RD);
    }
    else if(index==8){
        nand(RS,RT,RD);
    }
    else if(index==9){
        slt(RS,RT,RD);
    }
    else if(index==10){
        sll(RT,RD,SHAMT);
    }
    else if(index==11){
        srl(RT,RD,SHAMT);
    }
    else if(index=12){
        sra(RT,RD,SHAMT);
    }
    /**J-type instructions**/
    else if(index==13){
        jr(RS);
    }
    else if(index==14){
        jj(C);
    }
    else if(index==15){
        jal(C);
    }
    /**I-type instructions**/
    else if(index==16){
        addi(RS,RT,C);
    }
    else if(index==17){
        addiu(RS,RT,C);
    }
    else if(index==18){
        lw(RS,RT,signedC);
    }
    else if(index==19){
        lh(RS,RT,signedC);
    }
    else if(index==20){
        lhu(RS,RT,C);
    }
    else if(index==21){
        lb(RS,RT,signedC);
    }
    else if(index==22){
        lbu(RS,RT,C);
    }
    else if(index==23){
        sw(RS,RT,signedC);
    }
    else if(index==24){
        sh(RS,RT,signedC);
    }
    else if(index==25){
        sb(RS,RT,signedC);
    }
    else if(index==26){
        lui(RT,C);
    }
    else if(index==27){
        andi(RS,RT,C);
    }
    else if(index==28){
        or(RS,RT,C);
    }
    else if(index==29){
        nor(RS,RT,C);
    }
    else if(index==30){
        slti(RS,RT,C);
    }
    else if(index==31){
        beq(RS,RT,signedC);
    }
    else if(index==32){
        bne(RS,RT,signedC);
    }
    else if(index==33){
        bgtz(RS,signedC);
    }
    else{
        if(test==1) printf("this is error instruction\n");
    }
}

/**R-type instructions**/
void add(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }

    int j, carry=0, bitsum;
    int same=(REG[rs][0]==REG[rt][0])?1:0;
    char same_bit;

    if(same) same_bit=REG[rs-8][0];
    for(j=31; j>=0; j--){
        bitsum=(REG[rs][j]-'0')+(REG[rt][j]-'0')+carry;
        if(bitsum<2){
            REG[rd][j]=bitsum+'0';
            carry=0;
        }
        else{
            REG[rd][j]=bitsum-2+'0';
            carry=1;
        }
    }
    if(same){
        if(REG[rd][0]!=same_bit){
            number_overflow=1;
            return;
        }
    }
}
void addu(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }
    int j, carry=0, bitsum;

    for(j=31; j>=0; j--){
        bitsum=(REG[rs][j]-'0')+(REG[rt][j]-'0')+carry;
        if(bitsum<2){
            REG[rd][j]=bitsum+'0';
            carry=0;
        }
        else{
            REG[rd][j]=bitsum-2+'0';
            carry=1;
        }
    }
}
void sub(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }
    int j, carry=1, bitsum;
    int complement[32]={0};
    for(j=31; j>=0; j--) complement[j]=flip( REG[rt][j]-'0' );
    for(j=31; j>=0; j--){
        bitsum=complement[j]+carry;
        if(bitsum<2){
            complement[j]=bitsum;
            carry=0;
        }
        else{
            complement[j]=bitsum-2;
            carry=1;
        }
    }

    int same=( (REG[rs][0]-'0')==complement[0] ) ? 1:0;
    char same_bit;
    if(same) same_bit=REG[rs][0];
    carry=0;
    for(j=31; j>=0; j--){
        bitsum=(REG[rs][j]-'0')+complement[j]+carry;
        if(bitsum<2){
            REG[rd][j]=bitsum+'0';
            carry=0;
        }
        else{
            REG[rd][j]=bitsum-2+'0';
            carry=1;
        }
    }
    if(same){
        if(REG[rd][0]!=same_bit){
            number_overflow=1;
            PC_adder();
            return;
        }
    }
}
void and(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }

    int j, bitsum;
    if(rs==0 || rt==0){
        for(j=0; j<32; j++) REG[rd][j]=0;
        return;
    }
    for(j=31; j>=0; j--){
        bitsum=(REG[rs][j]-'0')&(REG[rt][j]-'0');
        REG[rd][j]=bitsum+'0';
    }
}
void or(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }

    int j, bitsum;
    for(j=31; j>=0; j--){
        bitsum=(REG[rs][j]-'0')|(REG[rt][j]-'0');
        REG[rd][j]=bitsum+'0';
    }
}
void xor(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }
    int j, bitsum;
    for(j=31; j>=0; j--){
        bitsum=(REG[rs][j]-'0')^(REG[rt][j]-'0');
        REG[rd][j]=bitsum+'0';
    }
}
void nor(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }
    int j, bitsum;
    for(j=31; j>=0; j--){
        bitsum=~((REG[rs][j]-'0')|(REG[rt][j]-'0'));
        REG[rd][j]=bitsum+'0';
    }
}
void nand(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }
    int j, bitsum;
    for(j=31; j>=0; j--){
        bitsum=~((REG[rs][j]-'0')&(REG[rt][j]-'0'));
        REG[rd][j]=bitsum+'0';
    }
}
void slt(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }

    int j, result;
    if(REG[rs][0]!=REG[rt][0]){ //different sign bit
        result=(REG[rt][0]=='0')?1:0;
        if(result){
            for(j=31; j>=0; j--){
                if(j==31) REG[rd][j]='1';
                else REG[rd][j]='0';
            }
        } else
            for(j=31; j>=0; j--) REG[rd][j]='0';
    }
    else{ //same sign bit
        for(j=0; j<32; j++){
            if(REG[rs][j]!=REG[rt][j]){
                result=(REG[rt][j]=='1')?1:0;
                break;
            }
        }
        if(result){
            for(j=31; j>=0; j--){
                if(j==31) REG[rd][j]='1';
                else REG[rd][j]='0';
            }
        } else
            for(j=31; j>=0; j--) REG[rd][j]='0';
    }
}
void sll(int rt, int rd, int shamt){
    if(rd==0 && rt==0 && shamt==0){ //NOP
        return;
    }
    if(rd==0){
        write$0_error=1;
        return;
    }

    int i;
    for(i=shamt; i<32; i++)
        REG[rd][i-shamt]=REG[rt][i];
    for(i=32-shamt; i<32; i++)
        REG[rd][i]='0';

}
void srl(int rt, int rd, int shamt){
    if(rd==0){
        write$0_error=1;
        return;
    }

    int i;
    for(i=0; i<32-shamt; i++)
        REG[rd][i+shamt]=REG[rt][i];
    for(i=0; i<shamt; i++)
        REG[rd][i]='0';

}
void sra(int rt, int rd, int shamt){
    if(rd==0){
        write$0_error=1;
        return;
    }

    int i;
    for(i=0; i<32-shamt; i++)
        REG[rd][i+shamt]=REG[rt][i];
    for(i=0; i<shamt; i++)
        REG[rd][i]=REG[rt][0];

}
void jr(int rs){
    int j;
    for(j=0; j<32; j++) PC_now[j]=REG[rs][j];
    PC_nowptr = char_BINtoDEC(PC_now, 32, 31);
}

/**J-type instructions**/
void jj(int c){
    int j=31;
    int digit=4*c, abin[34]={0};
    while(digit>0){
        abin[j]=digit%2;
        digit=digit/2;
        j--;
    }

    for(j=4; j<32; j++) PC_now[j]=abin[j]+'0';
    PC_nowptr = char_BINtoDEC(PC_now, 32, 31);
}
void jal(int c){
    int j=31;
    int digit=4*c, abin[34]={0};
    while(digit>0){
        abin[j]=digit%2;
        digit=digit/2;
        j--;
    }

    for(j=0; j<32;j++) REG[31][j]=PC_now[j];
    for(j=4; j<32; j++) PC_now[j]=abin[j]+'0';
    PC_nowptr = char_BINtoDEC(PC_now, 32, 31);
}

/**I-type instructions**/
void addi(int rs, int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }

    int j=31, carry=0, bitsum;
    //convert c from decimal to binary(sign extension)
    int digit=c, cbin[34]={0};
    while(digit>0){
        cbin[j]=digit%2;
        digit=digit/2;
        j--;
    }

    int same=(REG[rs][0]==cbin[0]+'0')?1:0;
    char same_bit;
    if(same) same_bit=REG[rs][0];
    for(j=31; j>=0; j--){
        bitsum=(REG[rs][j]-'0')+cbin[j]+carry;
        if(bitsum<2){
            REG[rt][j]=bitsum+'0';
            carry=0;
        }
        else{
            REG[rt][j]=bitsum-2+'0';
            carry=1;
        }
    }
    if(same){
        if(REG[rt][0]!=same_bit){
            number_overflow=1;
            PC_adder();
            return;
        }
    }
}
void addiu(int rs, int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }

    int j=31, carry=0, bitsum;
    //convert c from decimal to binary(sign extension)
    int digit=c, cbin[34]={0};
    while(digit>0){
        cbin[j]=digit%2;
        digit=digit/2;
        j--;
    }

    for(j=31; j>=0; j--){
        bitsum=(REG[rs][j]-'0')+cbin[j]+carry;
        if(bitsum<2){
            REG[rt][j]=bitsum+'0';
            carry=0;
        }
        else{
            REG[rt][j]=bitsum-2+'0';
            carry=1;
        }
    }
}
void lw(int rs, int rt, int c){
    if(rt==0) write$0_error=1;

    int result = char_BINtoDEC(REG[rs],32,31) + c;
    if(char_BINtoDEC(REG[rs],32,31)>0 && c>0 && result<0)
        number_overflow=1;
    if(result>255 || result<0)
        memory_overflow=1;

    int check = (REG[rs][31]-'0')+(REG[rs][30]-'0')*2+c;
    if(check%4!=0)
        misaligned=1;

    if(write$0_error==1 || memory_overflow==1 || misaligned==1) return;

    int i;
    for(i=0; i<32; i++){
        REG[rt][i]=dmemory[result][i];
    }
}
void lh(int rs, int rt, int c){
    if(rt==0) write$0_error=1;

    int result = char_BINtoDEC(REG[rs],32,31) + c;
    if(char_BINtoDEC(REG[rs],32,31)>0 && c>0 && result<0)
        number_overflow=1;
    if(result>255 || result<0)
        memory_overflow=1;

    int check = (REG[rs][31]-'0')+c;
    if(check%2!=0)
        misaligned=1;

    if(write$0_error==1 || memory_overflow==1 || misaligned==1) return;

    int i;
    for(i=0; i<16; i++){
        REG[rt][i+16]=dmemory[result][i+16];
    }

    if(dmemory[result][0]=='0'){
        for(i=0; i<16; i++) REG[rt][i]='0';
    }
    else{
        for(i=0; i<16; i++) REG[rt][i]='1';
    }
}
void lhu(int rs, int rt, int c){
    if(rt==0) write$0_error=1;

    int result = char_BINtoDEC(REG[rs],32,31) + c;
    if(char_BINtoDEC(REG[rs],32,31)>0 && c>0 && result<0)
        number_overflow=1;
    if(result>255 || result<0)
        memory_overflow=1;

    int check = (REG[rs][31]-'0')+c;
    if(check%2!=0)
        misaligned=1;

    if(write$0_error==1 || memory_overflow==1 || misaligned==1) return;

    int i;
    for(i=0; i<16; i++){
        REG[rt][i+16]=dmemory[result][i+16];
    }

    for(i=0; i<16; i++) REG[rt][i]='0';

}
void lb(int rs, int rt, int c){
    if(rt==0) write$0_error=1;

    int result = char_BINtoDEC(REG[rs],32,31) + c;
    if(char_BINtoDEC(REG[rs],32,31)>0 && c>0 && result<0)
        number_overflow=1;
    if(result>255 || result<0)
        memory_overflow=1;

    int check = (REG[rs][31]-'0')+c;
    if(check%2!=0)
        misaligned=1;

    if(write$0_error==1 || memory_overflow==1 || misaligned==1) return;

    int i;
    for(i=0; i<8; i++){
        REG[rt][i+24]=dmemory[result][i+24];
    }

    if(dmemory[result][0]=='0'){
        for(i=0; i<24; i++) REG[rt][i]='0';
    }
    else{
        for(i=0; i<24; i++) REG[rt][i]='1';
    }
}
void lbu(int rs, int rt, int c){
    if(rt==0) write$0_error=1;

    int result = char_BINtoDEC(REG[rs],32,31) + c;
    if(char_BINtoDEC(REG[rs],32,31)>0 && c>0 && result<0)
        number_overflow=1;
    if(result>255 || result<0)
        memory_overflow=1;

    int check = (REG[rs][31]-'0')+c;
    if(check%2!=0)
        misaligned=1;

    if(write$0_error==1 || memory_overflow==1 || misaligned==1) return;

    int i;
    for(i=0; i<8; i++){
        REG[rt][i+24]=dmemory[result][i+24];
    }

    for(i=0; i<24; i++) REG[rt][i]='0';

}
void sw(int rs, int rt, int c){
    int result = char_BINtoDEC(REG[rs],32,31) + c;
    if(char_BINtoDEC(REG[rs],32,31)>0 && c>0 && result<0)
        number_overflow=1;
    if(result>255 || result<0)
        memory_overflow=1;

    int check = (REG[rs][31]-'0')+(REG[rs][30]-'0')*2+c;
    if(check%4!=0)
        misaligned=1;

    if(memory_overflow==1 || misaligned==1) return;

    int i;
    for(i=0; i<32; i++){
        dmemory[result][i]=REG[rt][i];
    }
}
void sh(int rs, int rt, int c){
    int result = char_BINtoDEC(REG[rs],32,31) + c;
    if(char_BINtoDEC(REG[rs],32,31)>0 && c>0 && result<0)
        number_overflow=1;
    if(result>255 || result<0)
        memory_overflow=1;

    int check = (REG[rs][31]-'0')+(REG[rs][30]-'0')*2+c;
    if(check%4!=0)
        misaligned=1;

    if(memory_overflow==1 || misaligned==1) return;

    int i;
    for(i=0; i<16; i++){
        dmemory[result][i+16]=REG[rt][i+16];
    }

}
void sb(int rs, int rt, int c){
    int result = char_BINtoDEC(REG[rs],32,31) + c;
    if(char_BINtoDEC(REG[rs],32,31)>0 && c>0 && result<0)
        number_overflow=1;
    if(result>255 || result<0)
        memory_overflow=1;

    int check = (REG[rs][31]-'0')+(REG[rs][30]-'0')*2+c;
    if(check%4!=0)
        misaligned=1;

    if(memory_overflow==1 || misaligned==1) return;

    int i;
    for(i=0; i<8; i++){
        dmemory[result][i+24]=REG[rt][i+24];
    }
}
void lui(int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }

    //convert c from decimal to binary(16 bit)
    int j=15, digit=c, cbin[18]={0};
    while(digit>0){
        cbin[j]=digit%2;
        digit=digit/2;
        j--;
    }

    for(j=15; j>=0; j--) REG[rt][j]=cbin[j]+'0';
}
void andi(int rs, int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }

    int j;
    if(rs==0 || c==0){
        for(j=0; j<32; j++) REG[rt][j]='0';
        return;
    }

    //convert c from decimal to binary(sign extension)
    int digit=c, cbin[34]={0}, carry=0, bitsum; j=31;
    while(digit>0){
        cbin[j]=digit%2;
        digit=digit/2;
        j--;
    }

    for(j=31; j>=0; j--){
        bitsum=(REG[rs][j]-'0')&cbin[j];
        REG[rt][j]=bitsum+'0';
    }
}
void ori(int rs, int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }

    //convert c from decimal to binary(sign extension)
    int digit=c, cbin[34]={0}, carry=0, bitsum, j=31;
    while(digit>0){
        cbin[j]=digit%2;
        digit=digit/2;
        j--;
    }

    for(j=31; j>=0; j--){
        bitsum=(REG[rs][j]-'0')|cbin[j];
        REG[rt][j]=bitsum+'0';
    }
}
void nori(int rs, int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }

    //convert c from decimal to binary(sign extension)
    int digit=c, cbin[34]={0}, carry=0, bitsum, j=31;
    while(digit>0){
        cbin[j]=digit%2;
        digit=digit/2;
        j--;
    }

    for(j=31; j>=0; j--){
        bitsum=~((REG[rs][j]-'0')|cbin[j]);
        REG[rt][j]=bitsum+'0';
    }
}
void slti(int rs, int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }

    //convert c from decimal to binary(sign extension)
    int digit=c, cbin[34]={0}, j=31;
    while(digit>0){
        cbin[j]=digit%2;
        digit=digit/2;
        j--;
    }

    int result;
    if(REG[rs][0]!=(cbin[0]+'0')){ //different sign bit
        result=(cbin[0]==0)?1:0;
        if(result){
            for(j=31; j>=0; j--){
                if(j==31) REG[rt][j]='1';
                else REG[rt][j]='0';
            }
        }else
            for(j=31; j>=0; j--) REG[rt][j]='0';
    }
    else{ //same sign bit
        for(j=0; j<32; j++){
            if(REG[rs][j]!=(cbin[j]+'0')){
                result=(cbin[j]==1)?1:0;
                break;
            }
        }
        if(result){
            for(j=31; j>=0; j--){
                if(j==31) REG[rt][j]='1';
                else REG[rt][j]='0';
            }
        }else
            for(j=31; j>=0; j--) REG[rt][j]='0';
    }
}
void beq(int rs, int rt, int c){
    if(c>0 && (PC_nowptr + c)<0) number_overflow=1;

    int j, result=1;
    for(j=31; j>=0; j--){
        if(REG[rs][j]!=REG[rt][j]){
            result=0;
            break;
        }
    }
    if(result){
        PC_nowptr=PC_nowptr+c;
        j=31;
        int digit=PC_nowptr;
        while(digit>0){
            PC_now[j]=digit%2;
            digit=digit/2;
            j--;
        }
    }
}
void bne(int rs, int rt, int c){
    if(c>0 && (PC_nowptr + c)<0) number_overflow=1;

    int j, result=0;
    for(j=31; j>=0; j--){
        if(REG[rs][j]!=REG[rt][j]){
            result=1;
            break;
        }
    }
    if(result){
        PC_nowptr=PC_nowptr+c;
        j=31;
        int digit=PC_nowptr;
        while(digit>0){
            PC_now[j]=digit%2;
            digit=digit/2;
            j--;
        }
    }
}
void bgtz(int rs, int c){
    int j, result=0;
    for(j=0; j<32; j--){
        if(REG[rs][0]=='1'){ //is negative
            result=0;
            break;
        }
        if(REG[rs][j]=='1'){ //is positive and >0
            result=1;
            break;
        }
    }
    if(result){
        PC_nowptr=PC_nowptr+c;
        j=31;
        int digit=PC_nowptr;
        while(digit>0){
            PC_now[j]=digit%2;
            digit=digit/2;
            j--;
        }
    }
}


void memory(int index){
    if(index==0) return;
}

void write_back(int index){
    if(index==0) return;
}

/////////////////////////////////////////////

int main(){
    int i=0;
    initialize();
    snap=fopen("snapshot.rpt","w");
    error=fopen("error_dump.rpt","a");

    //cycle0
    instruction_fetch();
    initial_SNAP();

    //cycle1
    instruction_decode();
    instruction_fetch();
    append_SNAP();

    //cycle2
    execution(EX_index);


    while(1){
        write_back(WB_index);
        memory(DM_index);
        execution(EX_index);
        instruction_decode();
        instruction_fetch();

        append_SNAP();

        /**do error detection**/
        if(write$0_error){
            fprintf(error, "In cycle %d: Write $0 Error\n", cycle);
            write$0_error=0;
        }
        if(number_overflow){
            fprintf(error, "In cycle %d: Number Overflow\n", cycle);
            number_overflow=0;
        }
        if(memory_overflow){
            fprintf(error, "In cycle %d: Address Overflow\n", cycle);
            break;
        }
        if(misaligned){
            fprintf(error, "In cycle %d: Misalignment Error\n", cycle);
            break;
        }

        if(stall){
            WB_index=DM_index;
            DM_index=EX_index;
            EX_index=0;
            stall=0;
        } else{
            WB_index=DM_index;
            DM_index=EX_index;
            EX_index=ID_index;
        }
    }

    fclose(snap);
    fclose(error);
    return 0;
}

/////////////////////////////////////////////

/*int HEXtoDEC_bit(char c){
    if(c=='A') return 10;
    else if(c=='B') return 11;
    else if(c=='C') return 12;
    else if(c=='D') return 13;
    else if(c=='E') return 14;
    else if(c=='F') return 15;
    else return c-'0';
}*/
char DECtoHEX_bit(int n){
    if(n==10) return 'A';
    else if(n==11) return 'B';
    else if(n==12) return 'C';
    else if(n==13) return 'D';
    else if(n==14) return 'E';
    else if(n==15) return 'F';
    else return n+'0';
}
/*int char_HEXtoDEC(char arr[], int n_bits, int start){
    int i, digit, term, number=0, base=1;
    for(i=start; i>(start-n_bits); i--){
        digit=HEXtoDEC_bit(arr[i]);
        if(digit==0) term=0;
        else term=digit*base;
        number=number+term;
        base=base*16;
    }
    return number;
}
int int_HEXtoDEC(int arr[], int n_bits, int start){
    int i, digit, term, number=0, base=1;
    for(i=start; i>(start-n_bits); i--){
        digit=arr[i];
        if(digit==0) term=0;
        else term=digit*base;
        number=number+term;
        base=base*16;
    }
    return number;
}*/
int char_BINtoDEC(char arr[], int n_bits, int start){
    //if(test==1) printf("enter BINtoDEC(arr[0]=%c, %d bits star from %d)\n",arr[0],n_bits,start);
    int sum=0, base=1, i;
    for(i=start; i>(start-n_bits); i--){
        if(arr[i]=='1'){
            sum=sum+base;
        }
        base=base*2;
    }
    return sum;
}
int signed_char_BINtoDEC(char arr[], int n_bits, int start){
    int sum=0, base=1, i;
    for(i=start; i>(start-n_bits); i--){
        if(i==start-n_bits+1){
            if(arr[i]=='1') sum=sum-base;
        }
        else{
            if(arr[i]=='1') sum=sum+base;
            base=base*2;
        }
    }
    return sum;
}
int int_BINtoDEC(int arr[], int n_bits, int start){
    int sum=0, base=1, i;
    for(i=start; i>(start-n_bits); i--){
        sum=sum+arr[i]*base;
        base=base*2;
    }
    return sum;
}
int signed_int_BINtoDEC(int arr[], int n_bits, int start){
    int sum=0, base=1, i;
    for(i=start; i>(start-n_bits); i--){
        if(i==start-n_bits+1)
            sum=sum-arr[i]*base;
        else{
            sum=sum+arr[i]*base;
            base=base*2;
        }
    }
    return sum;
}
int flip(int n){
    return (n==0)?1:0;
}
