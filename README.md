# 🌻 Sunflower

> **STATUS:** Sunflower is early-stages work in progress! Feel free to make a pull request if you'd like to contribute :)

Sunflower is a language for procedurally generating graph structures. In Sunflower, you specify types of nodes, including what properties they have, and you specify constraints on the values of those properties. Then, the Sunflower compiler will take your script and randomly generate a graph structure that satisfies the constraints given.

**family_tree.sunflower**

```sunflower
ENUM Gender: MALE, FEMALE

DEF Person {
    gender: Gender
    mother: Person? // The question mark means the value can be NULL
    father: Person?
}

FOR Person x {
    x.mother.gender = FEMALE
    x.father.gender = MALE
}

COUNT Person[gender: FEMALE, mother: NULL, father: NULL] == 1
COUNT Person[gender: MALE, mother: NULL, father: NULL] == 1

COUNT Person[mother: NULL] == 2
COUNT Person[father: NULL] == 2
```

**console**

```
> sunflower family_tree.sunflower Person:10
```
