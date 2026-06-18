#ifndef HAZARDS_H
#define HAZARDS_H

typedef struct IF_ID IF_ID;
typedef struct ID_EX ID_EX;
typedef struct EX_MEM EX_MEM;

typedef enum {
    semHazard = 0,
    hazardDados = 1,
    hazardControle = 2
} tipoHazard;

tipoHazard unidadeDetecHazards(IF_ID *IF_ID, ID_EX *ID_EX, EX_MEM *EX_MEM);
tipoHazard detectaHazardDados(IF_ID *IF_ID, ID_EX *ID_EX, EX_MEM *EX_MEM);
tipoHazard detectaHazardControle(ID_EX *ID_EX, EX_MEM *EX_MEM);

#endif