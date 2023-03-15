# Escape The Cat

This is a solution for the Escape The Cat puzzle found [here](https://www.codingame.com/training/medium/escaping-the-cat "Escape The Cat").

By rotating and reflecting as needed, any position can be normalized so that the mouse lies on the x-axis with $x > 0$ and so that the cat lies in the upper half-plane.
From that point of view, it is clear that the evaluation of a position can be reduced to a function of two parameters. Namely, with $C$ and $M$ respectively denoting the cat's position and the mouse's position, let 
$$p_1 = d(0, M) \in [0, R]$$
and
$$p_2 = \theta(M, C) \in [0, \pi]$$
where $R$ is the radius of the pool and $\theta(a, b)$ denotes the angle formed at the origin by the two vectors $a$ and $b$.

Our solution goes as follows.
First, head towards the center.
Once the mouse is close to the center, at each turn we choose the direction on $\partial B(M, 10)$ which maximimizes our <em>score function</em>

$$\sigma(p_1, p_2) = p_1 / R + p_2 / \pi \in [0, 2].$$

The $p_2 / \pi$ term prioritizes directions which delay the cat, while the $p_1 / R$ term prioritizes directions which bring the mouse closer to the boundary.
I was expecting to have to associate weights to those two terms, possibly depending on $p_1$ and $p_2$, but their sum ended up being sufficient to solve all test cases. 

