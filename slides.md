---
author: Jonathan,
date: DD MMMM YYYY
paging: Slide %d / %d
---

## Interpretador de Expressões Lógicas

* O que é um interpretador?

Um interpretador é um programa de computador que executa diretamente as instruções fornecidas.

* E um interpretador de expressões lógicas?

> a ^ b -> logic

| a | b | CLRES |
| - | - |   -   |
| 1 | 1 |     1 |
| 1 | 0 |     0 |
| 0 | 1 |     0 |
| 0 | 0 |     0 |

---

## Exemplos

* Conjunção: a ^ b
* Disjunção: a v b
* Disjunção Exclusiva: a + b
* Condicional: a -> b
* Bicondicional: a <-> b
* Expressão complexa: (r -> (b + a))

---

## Arquitetura

```
Entrada               Lexer           Parser          Saída
┌──────────┐  Chars   ┌───┐  Tokens   ┌───┐   Stack   ┌────┐
│(,a,^ ... │ ───────> │ L │ ────────> │ P │ ────────> │ TV │
└──────────┘          └───┘           └───┘           └────┘
```
```c
struct Token {
    char simbolo[];
    Token_Type tipo;
};

{"->", BINOP}        // Token operação binaria
{"(", TOKEN_OPPAREN} // Token abre parenteses
{"a", TOKEN_PREP}    // Token preposição
```

---

## Exemplo de Execução

```
Entrada: a -> b
Stack = [ ]
a = V
b = V

┌ 1 ───────────┐  ┌ 2 ───────────┐ ┌ 3 ─────────────────┐
│ a -> b       │  │ a -> b       │ │ a -> b             │
│ ^            │  │   ^          │ │      ^             │
│              │  │              │ │                    │
│ push(&S, a); │  │ v = pop(&S); │ │ res = CONDS(v, b); │
│              │  │              │ │ push(&S, res);     │
│              │  │              │ │                    │
│  S           │  │  S           │ │    S               │
│ [V]          │  │ [ ] -> Head  │ │   [V]              │
│ [ ] -> Head  │  │              │ │   [ ] -> Head      │
└──────────────┘  └──────────────┘ └────────────────────┘
```

---

## Exemplo de Execução

```
                    1   2   3   4
1. a = V, b = V    [V] [V] [V] [V]
2. a = V, b = F        [F] [F] [F]
3. a = F, b = V            [V] [V]
4. a = F, b = F                [V]
```

| a   | b   | a -> b |
| :-: | :-: |   :-:  |
| 1   | 1   |    1   |
| 1   | 0   |    0   |
| 0   | 1   |    1   |
| 0   | 0   |    1   |

---

## Bibliografia

* Compiladores: Princípios, Técnicas e Ferramentas
* The C Programming Language
* www.youtube.com/tsoding