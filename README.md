# Projection Iterator Adaptor

## License

Everything in this project is authored by me (Joe Zbiciak,
joe.zbiciak@leftturnonly.info), and is licensed under the Creative Commons
Attribution-ShareAlike 4.0 International license, aka.
[CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/).

## Background

Awhile back, someone on Quora posed the question of how to sort an array
such that the first half of the sorted elements would be in the even
positions of the output array in increasing order, and the remaining elements
would be in the odd elements of the output array in decreasing order.

That is, if you were given:

```
3, 9, 6, 0, 1, 5, 4, 7, 2, 8
```

The ordinary sorted view would be:

```
0, 1, 2, 3, 4, 5, 6, 7, 8, 9
```

But the desired view is actually:

```
0, 9, 1, 8, 2, 7, 3, 6, 4, 5
```

And, of course, if you think of the problem more generically, the input isn't
guaranteed to be dense integers.  It's not even guaranteed to be integers.

My approach: Construct an interator that projects a different view of the
container, and sort the projected view of the container.  We can use the
standard sort, and it never needs to know we transformed its view of the
underlying container.

This was brought up as an off-the-cuff idea on David Seidman's answer to an
interviewing-related question [here.](https://www.quora.com/What-are-some-signs-right-away-that-the-software-engineering-candidate-youre-interviewing-at-a-company-like-Google-Microsoft-and-Facebook-wont-make-it/answer/David-SeidmanA)

Bruce Miller posed the idea of overloading `operator[]`; however that's not
enough if you want to use `std::sort`.  I don't remember whether I had already
come up with the iterator idea or not by the time I saw Bruce's comment.

Amusingly, David, who also worked for my employer at the time, suggested I try
to get a job there.  "Psst... I already work here."  `:-D`

## The Adaptor: How It Works

The projection adaptor takes a base random-access iterator and a projection
function.

Internally the adaptor keeps track of the linear deltas that have been applied
to the iterator. To produce a concrete adaptor, it takes the accumulated deltas,
calls the supplied projection function, and then uses `std::advance` to create
a concrete iterator that points to the projected element.

_Note:_ I call `std::advance` because at some point I would like to upgrade
this to support other iterator categories.  It won't be as efficient as random
access iterators, and it may not even meet the performance guarantees of the
base iterator's category, but it would still _work._

Most of the code is just boilerplate to fill C++'s expectations of iterators.
Beyond that, it isn't particularly fancy.  It does return a data type whose
name is unutterable outside the `make_permutation_iterator` function, though.

It's a toy, but it's a fun and instructive toy.  I've tried to define
everything in a manner that would allow this to be used for meaningful work.
Originally I targeted C++17, but then I realized it wouldn't be difficult to
target C++14 with minor tweaks.  Therefore, this code requires C++14 at a
minimum.  I left C++11 out of scope.

____

Copyright ?? 2023, Joe Zbiciak <joe.zbiciak@leftturnonly.info>  
`SPDX-License-Identifier:  CC-BY-SA-4.0`
