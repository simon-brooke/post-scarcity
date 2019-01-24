# Mad software

I was listening to [Eric Normand's podcast](https://lispcast.com/tension-between-data-and-entity/) this morning, as I was making breakfast and tidying my room; he was talking about semantics and data. It started a train of thought which I shall try to unroll.

I have blogged a lot in the past about madness and about software, but I don't think I've ever blogged about madness and software in the same essay. But the reasons I'm mad and the reasons I'm (sometimes) very good at software are related; both have their roots in autism and dyslexia, or, to put it differently, how my brain is wired.

I first wrote about [post scarcity software](https://blog.journeyman.cc/2006/02/post-scarcity-software.html) thirteen years ago. It was a thought about how software environments should be designed if were weren't held back by the cruft of the past, by tradition and by a lack, frankly, of anything much in the way of new creative thought. And seeing that the core of the system I described is a Lisp, which is to say it builds on a software architecture which is exactly as old as I am, perhaps it is infected by my take on tradition and my own lack of creativity, but let's, for the purposes of this essay, assume not.

I started actually writing the [post scarcity software environment](https://github.com/simon-brooke/post-scarcity) on the second of January 2017, which is to say two years ago. It's been an extremely low priority task, because I don't have enough faith in either my vision or my skill to think that it will ever be of use to anyone. Nevertheless, it does now actually work, in as much as you can write software in it. It's not at all easy yet, and I wouldn't recommend anyone try, but you can check out the master branch from Github, compile it, and it works.

As my mental health has deteriorated, I have been working on it more over the past couple of months, partly because I have lost faith in my ability to deliver the more practical projects I've been working on, and partly because doing something which is genuinely intellectually hard helps subdue the chaos in my mind.

Having said that, it is hard and I am not sharp, and so progress is slow. I started work on big number arithmetic a three weeks ago, and where I'm up to at this point is:

* addition seems to work up to at least the second bignum boundary;
* multiplication doesn't work beyond the first bignum boundary;
* subraction doesn't work, and turns out not to be as easy as just inverting addition;
* division sort of trivially works, but only in the sense that we can create a rational number out of arbitrary bignums;
* reading works beyond the first bignum boundary, but not up to the second (because multiplication doesn't work);
* printing doesn't work beyond the first bignum boundary.

I knew bignums were going to be a challenge, and I could have studied other people's bignum code and have consciously chosen not to do so; but this is not fast progress.

(I should point out that in those three weeks I've also done four days of customer work, which is .Net and boring but it's done, spent two days seeing my sister, spent two days so depressed I didn't actually do anything at all, and done a bit or practical work around the croft. But still!)

In a sense, it wasn't expected to be. Writing the underpinnings of a software environment which is conceptually without limits has challenge after challenge after challenge.

But there are ideas in post scarcity which may have wider utility than this mad idea in itself. Layering homogeneities and regularities onto Clojure maps might - perhaps would - make a useful library, might would make a very useful component for exactly the sort of data wrangling Eric Normand was talking about. Yes, you can use a map - raw data soup - to represent a company. But if this map is a member of a homogeneity, 'Companies', then we know every member of it has employees, and that every employee has a salary and an email address. Regularities and homogeneities form the building blocks of APIs; to use the example Eric discussed in his podcast, the salary is the property of the employee, but the payroll is a property of the company. So in post scarcity, you'd get the payroll figure for a company by using a method on the 'Companies' homogeneity. How it computes that value is part of the general doctrine of **'Don't Know, Don't Care'**: the principal that people writing software at any layer in the system do not need to know, and should not need to care, about how things are implemented in the layers below them.



So, the user needing to find the payroll value would enter something like this:

```
    (with ((companies . ::shared:pool:companies)
           (acme . companies:acme-widgets))
        (companies:methods:payroll acme))
```

In practice, in post scarcity notation, the payroll method probably looks something like this:

```
    (lambda (company)
        (reduce + (map ::shared:pool:employees:methods:salary (:employees company))))
```

There are issues that I haven't resolved yet about the mutability of regularities and homogeneities; obviously, in order to provide multi-user visibility of current values of shared data, some regularities must be mutable. But mutability has potentially very serious perfomance issues for the hypercube architecture, so I think that in general they should not be.

However, that's detail, and not what I'm trying to talk about here.

What I'm trying to talk about here is the fact that if I were confident that these ideas were any good, and that I had the ability to persuade others that they were any good, it would make far more sense to implement them in Clojure and promote them as a library.

But the problem with depression is that you cannot evaluate whether your ideas are any good. The black dog tells you you're shit, and that your ideas are shit, and that you don't really know enough to be worth listening to, and that you're an old tramp who lives in a hut in the woods, and probably smells, and that in any case interaction with other people quickly makes you shaky and confused, and that you can never get your act together, and you never finish anything.

And all that is objectively true, and I know that it is true. But I also know that I can (or at least have in the past been able to) build really good software, and that I can (or have been able, in the past, to) present ideas really well.

These two collections of statements about me are both true at the same time. But the difference is that I believe the first and I don't believe the second.

And behind all this is the fact that bignum arithmetic is a solved problem. I could dig out the SBCL source code and crib from that. I am bashing my head against bignum arithmetic and trying to solve it myself, not because it's the most efficient way to produce good code quickly, but because what I'm really trying to do is just distract myself and waste time while I can get on with dying.

And the reason beyond that that I'm working on a software system I know I'll never finish, which is designed to run on computers which don't even exist yet - and although I'm very confident that enormously parallel hardware will be used in future, I'm not at all sure it will look anything like what I'm envisaging - the reason I'm building this mad software is that, because it will never be finished, no-one will ever use it except me, and no-one will say how crap it is and how easily it could have been done better.

Because the other thing that I'm doing in writing this stuff, apart from distracting from the swirling chaos and rage in my head, apart from waiting to die, the other thing I'm doing is trying to give myself a feeling of mastery, of competence, of ability to face problems and solve them. And, to an extent, it works. But I have so little confidence that I actually have that mastery, that competence, that I don't want to expose it to criticism. I don't want my few fragile rags of self worth stripped away.

And so I work, and work, and work at something which is so arcane, so obscure, so damned pointless that no-one will ever use it.

Not because I'm even enjoying it, but just to burn the time.

This is mad.

I am mad.

I hate, hate, hate being mad.

Postscript: just writing this essay has made me tearful, headachey, sick, shaky. It's very hard to face up to the irrationalities and self-deceptions in one's own behaviour.
