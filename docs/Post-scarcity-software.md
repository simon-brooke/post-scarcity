_This is the text of my essay Post-scarcity Software, originally published in 2006 on my blog [here](http://blog.journeyman.cc/2006/02/post-scarcity-software.html)._

For years we've said that our computers were Turing equivalent, equivalent to Turing's machine U. That they could compute any function which could be computed. They aren't, of course, and they can't, for one very important reason. U had infinite store, and our machines don't. We have always been store-poor. We've been mill-poor, too: our processors have been slow, running at hundreds, then a few thousands, of cycles per second. We haven't been able to afford the cycles to do any sophisticated munging of our data. What we stored - in the most store intensive format we had - was what we got, and what we delivered to our users. It was a compromise, but a compromise forced on us by the inadequacy of our machines.

The thing is, we've been programming for sixty years now. When I was learning my trade, I worked with a few people who'd worked on Baby - the Manchester Mark One - and even with two people who remembered Turing personally. They were old then, approaching retirement; great software people with great skills to pass on, the last of the first generation programmers. I'm a second generation programmer, and I'm fifty. Most people in software would reckon me too old now to cut code. The people cutting code in the front line now know the name Turing, of course, because they learned about U in their first year classes; but Turing as a person - as someone with a personality, quirks, foibles - is no more real to them than Christopher Columbus or Noah, and, indeed, much less real than Aragorn of the Dunedain.

In the passing generations we've forgotten things. We've forgotten the compromises we've made; we've forgotten the reasons we've made them. We're no longer poor. The machine on which I'm typing this - my personal machine, on my desk, used by no-one but me - has the processor power of slightly over six thousand DEC VAXes; it has the one hundred and sixty two thousand times as much core store as the ICL 1900 mainframe on which I learned Pascal. Yet both the VAX and the 1900 were powerful machines, capable of supporting dozens of users at the same time. Compared to each individual user of the VAX, of the 1900, I am now incalculably rich. Vastly. Incomprehensibly.

And it's not just me. With the exception of those poor souls writing embedded code for micro-controllers, every programmer now working has processor and store available to him which the designers of the languages and operating systems we still use could not even have dreamed of. UNIX was designed when 32 bit machines were new, when 16,384 bytes was a lot of memory and very expensive. VMS - what we now call 'Windows XP' - is only a few years younger. 

The compromises of poverty are built into these operating systems, into our programming languages, into our brains as programmers; so deeply ingrained that we've forgotten that they are compromises, we've forgotten why we chose them. Like misers counting grains on the granary floor while outside the new crop is falling from the stalks for want of harvesting, we sit in the middle of great riches and behave as though we were destitute.

One of the things which has made this worse in recent years is the rise of Java, and, following slavishly after it, C#. Java is a language which was designed to write programs for precisely those embedded micro-controllers which are still both store and mill poor. It is a language in which the mind-set of poverty is consciously ingrained. And yet we have adopted it as a general purpose programming language, something for which it is not at all suitable, and in doing so have taught another generation of programmers the mind-set of poverty. Java was at least designed; decisions were made for reasons, and, from the point of view of embedded micro-controllers, those reasons were good. C# is just a fit of pique as software. Not able to 'embrace and extend' Java, Microsoft aped it as closely as was possible without breaching Sun's copyright. Every mistake, every compromise to poverty ingrained in Java is there in C# for all the world to see.

It's time to stop this. Of course we're not as wealthy as Turing. Of course our machines still do not have infinite store. But we now have so much store - and so many processor cycles - that we should stop treating them as finite. We should program as if we were programming for U.

# Store, Name and Value

So let's start with what we store, what we compute on: values. For any given column within a table, for every given instance variable in a class, every record, every object is constrained to have a value with a certain format.

This is, of course, historical. Historically, when storage was expensive we stored textual values in fields of fixed width to economise on storage; we still do so largely because that's what we've always done rather than because there's any longer any rational reason to. Historically, when storage and computation were expensive, we stored numbers in twos-complement binary strings in a fixed number of bytes. That's efficient, both of store and of mill.

But it is no longer necessary, nor is it desirable, and good computer languages such as LISP transparently ignores the difference between the storage format of different numbers. For example:

    (defun factorial (n)
      (cond 
        ((eq n 1) 1)
        (t (* n (factorial (- n 1))))))

    ;; a quick way to generate very big numbers...

We can add the value of factorial 100 to an integer, say 2, in just the same way that we can add any other two numbers:

    (+ (fact 100) 2)
    933262154439441526816992388562667004907159682643816214685929638952175999932299156089414639761565182862536979208272237582511852109168 64000000000000000000000002

We can multiply the value of factorial 100 by a real number, say pi, in just the same way as we can add any other two numbers:

    (* (factorial 100) pi)
    2.931929528260332*10^158

The important point to note here is that there's no explicit call to a bignum library or any other special coding. LISP's arithmetic operators don't care what the underlying storage format of a number is, or rather, are able transparently to handle any of the number storage formats - including bignums - known to the system. There's nothing new about this. LISP has been doing this since the late 1960s. Which is as it should be, and, indeed, as it should be in storage as well as in computation.

A variable or a database field (I'll treat the two as interchangeable, because, as you will see, they are) may reasonably have a validation rule which says that a value which represents the longitude of a point on the Earth in degrees should not contain a value which is greater than 360. That validation rule is domain knowledge, which is a good thing; it allows the system to have some vestige of common sense. The system can then throw an exception when it is asked to store 764 as the longitude of a point, and this is a good thing.

Why then should a database not throw an exception when, for example, a number is too big to fit in the internal representation of a field? To answer, here's a story I heard recently, which seems to be apocryphal, but which neatly illustrates the issue just the same.

_The US Internal Revenue Service have to use a non-Microsoft computer to process Bill Gate's income tax, because Microsoft computers have too small an integer representation to represent his annual income._

Twos complement binary integers stored in 32 bits can represent plus or minus 2,147,483,648, slightly over two US billion. So it's easily possible that Bill Gates' income exceeds this. Until recently, Microsoft operating systems ran only on computers with a register size of 32 bits. Worryingly, the default integer size of my favourite database, Postgres, is also 32 bits.

This is just wrong. Nothing in the domain of income places any fixed upper bound on the income a person may receive. Indeed, with inflation, the upper limit on incomes as quantity is likely to continue to rise. Should we patch the present problem by upping the size of the integer to eight bytes?

In Hungary after the end of World War II inflation ran at 4.19 * 10^16 percent per month - prices doubled every 15 hours. Suppose Gates' income in US dollars currently exceeds the size of a thirty two bit integer, it would take at most 465 hours - less than twenty days - to exceed US$9,223,372,036,854,775,808. What's scary is how quickly you'd follow him. If your present annual salary is just thirty three thousand of your local currency units, then given that rate of inflation, you would overflow a sixty-four bit integer in just 720 hours, or less than a month.

Lots of things in perfectly ordinary domains are essentially unbounded. They aren't shorts. They aren't longs. They aren't doubles. They're numbers. And a system asked to store a number should store a number. Failure to store a number because it's size violates some constraint derived from domain knowledge is desirable behaviour; failure to store a number because it size violates the internal storage representation of the system is just bad, outdated, obsolete system design. Yes, it's efficient of compute power on thirty-two bit processors to store values in thirty-two bit representations. Equally, it's efficient of disk space for a database to know in advance just how mush disk it has to reserve for each record in a table, so that to skip to the Nth record it merely has to skip forward (N * record-size) bytes.

But we're no longer short of either processor cycles or disk space. For a database to reject a value because it cannot be stored in a particular internal representation is industrial archaeology. It is a primitive and antiquated workaround from days of hardware scarcity. In these days of post-scarcity computing, it's something we should long have forgotten, long have cast aside.

This isn't to say that integers should never be stored in thirty-two bit twos complement binary strings. Of course they should, when it's convenient to do so. It's a very efficient storage representation. Of course, when a number overflows a thirty two bit cell, the runtime system has got to throw an exception, has got to deal with it, and consequently the programmer who writes the runtime system has still got to know about and understand the murky aspects of internal storage formats.

Perhaps the language designer, and the programmer who writes the language compiler should, too, but personally I don't think so. I think that at the layer in the system - the level of abstraction - at which the compiler writer works, the operator 'plus' should just be a primitive. It takes two numbers, and returns a number. That's all. The details of whether that's a float, a double, a rational or a bignum should not be in the least relevant at the level of language. There is a difference which is important between a real number and an integer. The old statistical joke about the average family having 2.4 children is funny precisely because it violates our domain knowledge. No family has 2.4 children. Some things, including children, are discrete, however indiscreet you may think them. They come in integral quantities. But they don't come in short quantities or long quantities. Shorts and longs, floats and doubles are artefacts of scarcity of store. They're obsolete.

From the point of view of the runtime designer, the difference between a quantity that can be stored in two bytes, or four, or eight must matter. From the point of view of the application designer, the language designer, even the operating system designer, they should disappear. An integer should be an integer, whether it represents the number of toes on your left foot (about 5), the number of stars in the galaxy (about 1x1011) or the number of atoms in the universe (about 1x1079). Similarly, a real number should be just a real number.

This isn't to say we can't do data validation. It isn't to say we can't throw a soft exception - or even a hard one - when a value stored in a variable or field violates some expectation, which may be an expectation about size. But that should be an expectation based on domain knowledge, and domain knowledge alone; it should not be an expectation based on implementation knowledge.

Having ranted now for some time about numbers, do you think I'm finished? I'm not. We store character values in databases in fields of fixed size. How big a field do we allocate for someone's name? Twenty four characters? Thirty-two? We've all done it. And then we've all found a person who violates our previous expectation of the size of a name, and next time we've made the field a little bigger. But by the time we've made a field big enough to store Charles Philip Arthur George Windsor or Sirimavo Ratwatte Dias Bandaranaike we've negated the point of fixed width fields in the first place, which was economy. There is no natural upper bound to the length of a personal name. There is no natural upper bound to the length of a street address. Almost all character data is a representation at some level of things people say, and the human mind doesn't work like that.

Of course, over the past fifty years, we've tried to make the human mind work like that. We've given addresses standardised 'zip codes' and 'postcodes', we've given people standardised 'social security numbers' and 'identity codes'. We've tried to fit natural things into fixed width fields; we've tried to back-port the inadequacies of our technology onto the world. It's stupid, and it's time we stopped.

So how long is a piece of string? How long is a string of characters? It's unbounded. Most names are short, because short names are convenient and memorable. But that does not mean that for any given number of characters, it's impossible that there should be something with a normal name of that length. And names are not the only things we store in character strings. In character strings we store things people say, and people talk a lot.

At this point the C programmers, the Java programmers are looking smug. Our strings, they say, are unbounded. Sorry lads. A C string is a null terminated sequence of bytes. It can in principle be any length. Except that it lives in a malloced lump of heap (how quaint, manually allocating store) and the maximum size of a lump of heap you can malloc is size_t, which may be 231, 232, 263 or 264 depending on the system. Minus one, of course, for the null byte. In Java, similarly, the size of a String is an int, and an int, in Java, means 231. 

Interestingly, Paul Graham, in his essay 'The Hundred YearLanguage', suggests doing away with stings altogether, and representing them as lists of characters. This is powerful because strings become S-expressions and can be handled as S-expressions; but strings are inherently one-dimensional and S-expressions are not. So unless you have some definite collating sequence for a branching 'string' it's meaning may be ambiguous. Nevertheless, in principle and depending on the internal representation of a CONS cell, a list of characters can be of indefinite extent, and, while it isn't efficient of storage, it is efficient of allocation and deallocation; to store a list of N characters does not require us to have a contiguous lump of N bytes available on the heap; nor does it require us to shuffle the heap to make a contiguous lump of that size available.

So; to reprise, briefly.

A value is just a value. The internal representation of a value is uninteresting, except to the designer and author of the runtime system - the virtual machine. For programmers at every other level the internal representation of every value is DKDC: don't know, don't care. This is just as true of things which are fundamentally things people say, things which are lists and things which are pools, as it is of numbers. The representation that the user - including the programmer - deals with is the representation which is convenient and comfortable. It does not necessarily have anything to do with the storage representation; the storage representation is something the runtime system deals with, and that the runtime system effectively hides. Operators exposed by the virtual machine are operators on values. It is a fundamental error, a failure of the runtime designer's most basic skill and craft, for a program ever to fail because a value could not be represented in internal representation - unless the store available to the system is utterly exhausted.

# Excalibur and the Pool

A variable is a handle in a namespace; it gives a name to a value, so that we can recall it. Storing a value in a variable never causes an exception to be thrown because the value cannot be stored. But it may, reasonably, justifiably, throw an exception because the value violates domain expectations. Furthermore, this exception can be either soft or hard. We might throw a soft exception if someone stored, in a variable representing the age of a person in years, the value 122. We don't expect people to reach one hundred and twenty two years of age. It's reasonable to flag back to whatever tried to set this value that it is out of the expected range. But we should store it, because it's not impossible. If, however, someone tries to store 372 in a variable representing longitude in degrees, we should throw a hard exception and not store it, because that violates not merely a domain expectation but a domain rule.

So a variable is more than just a name. It is a slot: a name with some optional knowledge about what may reasonably be associated with itself. It has some sort of setter method, and possibly a getter method as well.

I've talked about variables, about names and values. Now I'll talk about the most powerful abstraction I use - possibly the most powerful abstraction in software - the namespace. A namespace is a sort of pool into which we can throw arbitrary things, tagging each with a distinct name. When we return to the pool and invoke a name, the thing in the pool to which we gave that name appears.

## Regularities: tables, classes, patterns

Database tables, considered as sets of namespaces, have a special property: they are regular. Every namespace which is a record in the same table has the same names. A class in a conventional object oriented language is similar: each object in the class has the same set of named instance variables. They match a pattern: they are in fact constrained to match it, simply by being created in that table or class.

Records in a table, and instance variables in a class, also have another property in common. For any given name of a field or instance variable, the value which each record or object will store under that name is of the same type. If 'Age' is an integer in the definition of the table or class, the Age of every member will be an integer. This property is different from regularity, and, lacking a better word for it, I'll call it homogeneity. A set of spaces which are regular (i.e. share the same names) need not be homogeneous (i.e. share the same value types for those names), but a set which is homogeneous must be regular.

But records in a table, in a view, in a result set are normally in themselves values whose names are the values of the key field. And the tables and views, too, are values in a namespace whose names are the table names, and so on up. Namespaces, like Russian dolls, can be nested indefinitely. By applying names to the nested spaces at each level, we can form a path of names to every space in the meta-space and to each value in each space, provided that the meta-space forms an acyclic directed graph (this is, after all, the basis of the XPath language. Indeed, we can form paths even if the graph has cycles, provided every cycle in the graph has some link back to the root.

## Social mobility

It's pretty useful to gather together all objects in the data space which match the same pattern; it's pretty useful for them all to have distinct names. So the general concept of a regularity which is itself a namespace is a useful one, even if the names have to be gensymed.

To be in a class (or table), must a space be created in that class (or table)? I don't see why. One of my earlier projects was an inference engine called Wildwood, in which objects inferred their own class by exploring the taxonomy of classes until they found the one in which they felt most comfortable. I think this is a good model. You ought to be able to give your dataspace a good shake and then pull out of it as a collection all the objects which match any given pattern, and this collection ought to be a namespace. It ought to be so even if the pattern did not previously exist in the data space as the definition of a table or class or regularity or whatever you care to call it. 

A consequence of this concept is that objects which acquire new name-value pairs may move out of the regularity in which they were created either to exist as stateless persons in the no-man's land of the dataspace, or into a new regularity; or may form the seed around which a new regularity can grow. An object which acquires a value for one of its names which violates the validation constraints of one homogeneity may similarly move out into no-man's land or into another. In some domains, in some regularities, it may be a hard error to do this (i.e. the system will prevent it). In some domains, in some regularities, it may be a soft error (i.e. the system allows it under protest). In some domains, in some regularities, it may be normal; social mobility of objects will be allowed.

## Permeability

There's another feature of namespaces which gets hard wired into lots of software structures without very often being generalised, and that is permeability, semi-translucency. In my toolkit Jacquard, for example, values are first searched for in the namespace of http parameters; if not found there, in the namespace of cookies; next, in the namespace of session variables, then in local configuration parameters, finally in global configuration parameters. There is in effect a layering of semi-translucent namespaces like the veils of a dancer. 

It's not a pattern that's novel or unique to Jacquard, of course. But in Jacquard it's hard wired and in all the other contexts in which I've seen this pattern it's hardwired. I'd like to be able to manipulate the veils; to add, or remove, of alter the layering. I'd like this to be a normal thing to be able to do. 
The Name of the Rose: normativeness and hegemony
I have a friend called Big Nasty. Not everyone, of course, calls him Big Nasty. His sons call him 'Dad'. His wife calls him 'Norman'. People who don't know him very well call him 'Mr Maxwell'. He does not have one true name.

The concept of a true name is a seductive one. In many of the traditions of magic - and I have always seen software as a technological descendant or even a technological implementation of magic - a being invoked by its true name must obey. In most modern programming languages, things tend to have true names. There is a protocol for naming Java packages which is intended to guarantee that every package written anywhere in the world has a globally unique true name. Globally unique true names do then have utility. It's often important when invoking something to be certain you know exactly what it is you're invoking.

But it does not seem to me that this hegemonistic view of the dataspace is required by my messy conception. Certainly it cannot be true that an object has only one true name, since it may be the value of several names within several spaces (and of course this is true of Java; a class well may have One True Name, but I can still create an instance variable within an object whose name is anythingILike, and have its value is that class).

The dataspace I conceive is a soup. The relationships between regularities are not fixed, and so paths will inevitably shift. And in the dataspace, one sword can be in many pools - or even many times in the same pool, under different names - at the same time. We can shake the dataspace in different ways to see different views on the data. There should be no One True hegemonistic view. 

This does raise the question, 'what is a name'. In many modern relational databases, all primary keys are abstract and are numbers, even if natural primary keys exist in the data - simply because it is so easy to create a table with an auto-incrementer on the key field. Easy, quick, convenient, lazy, not always a good thing. In terms of implementation details, namespaces are implemented on top of hash tables, and any data object can be hashed. So can anything be a name?

In principle yes. However, my preference would be to purely arbitrarily say no. My preference would be to say that a name must be a 'thing people say', a pronounceable sequence of characters; and also, with no specific upper bound, reasonably short.

## The Problem with Syntax

Let me start by saying that I really don't understand the problem with syntax. Programming language designers spend a lot of time worrying about it, but I believe they're simply missing the point. People say 'I can't learn LISP because I couldn't cope with all the brackets'. People - the Dylan team, for one - have developed systems which put a skin of 'normal' (i.e., ALGOL-like) syntax on top of LISP. I personally won't learn Python because I don't trust a language where white space is significant. But in admitting that prejudice I'm admitting to a mistake which most software people make.

We treat code as if it wasn't data. We treat code as if it were different, special. This is the mistake made by the LISP2 brigade, when they gave their LISPs (ultimately including Common LISP) separate namespaces, one for 'code' and one for 'data'. It's a fundamental mistake, a mistake which fundamentally limits our ability to even think about software.

What do I mean by this?

Suppose I ask my computer to store pi, 3.14159265358979. Do I imagine that somewhere deep within the machine there is a bitmap representation of the characters? No, of course I don't. Do I imagine there's a vector starting with the bytes 50 46 49 51 49 53 57 ...? Well, of course, there might be, but I hope there isn't because it would be horribly inefficient. No, I hope and expect there's an IEEE 754 binary encoding of the form 01100100100001111...10. But actually, frankly, I don't know, and I don't care, provided that it is stored and that it can be computed with.

However, as to what happens if I then ask my computer to show me the value it has stored, I do know and I do care. I expect it to show me the character string '3.14159265358979' (although I will accept a small amount of rounding error, and I might want it to be truncated to a certain number of significant figures). The point is, I expect the computer to reflect the value I have stored back to me in a form which it is convenient for me to read, and, of course, it can.

We don't, however, expect the computer to be able to reflect back an executable for us in a convenient form, and that is in itself a curious thing. If we load, for example, the UNIX command 'ls' into a text editor, we don't see the source code. We see instead, the raw internal format. And the amazing thing is that we tolerate this.

It isn't even that hard to write a 'decompiler' which can take a binary and reflect back source code in a usable form. Here, for example, is a method I wrote:

    /**
     * Return my action: a method, to allow for specialisation. Note: this
     * method was formerly 'getAction()'; it has been renamed to disambiguate
     * it from 'action' in the sense of ActionWidgets, etc.
     */
    public String getNextActionURL( Context context ) throws Exception
    {
        String nextaction = null;

        HttpServletRequest request =
            (HttpServletRequest) context.get( REQUESTMAGICTOKEN );

        if ( request != null )
        {
            StringBuffer myURL = request.getRequestURL(  );

            if ( action == null )
            {
                nextaction = myURL.toString(  );

                // If I have no action, default my action
                // to recall myself
            }
            else
            {
                nextaction =
                    new URL( new URL( myURL.toString(  ) ), action ).toString(  );

                // convert my action into a fully
                // qualified URL in the context of my
                // own
            }
        }
        else
        { // should not happen!
            throw new ServletException( "No request?" );
        }

        return nextaction;
    }

and here is the result of 'decompiling' that method with an open-source Java decompiler, jreversepro:

    public String getNextActionURL(Context context)
                throws Exception
    {
         Object object = null;
         HttpServletRequest httpservletrequest = 
              (HttpServletRequest)context.get( "servlet_request");
         String string;
         if (httpservletrequest != null) {
              StringBuffer stringbuffer = httpservletrequest.getRequestURL();
              if (action == null)
                   string = stringbuffer.toString();
              else
                   string = new URL(new URL(stringbuffer.toString()) ,
                                    action).toString();
         }
         else
              throw new ServletException("No request?");

         return (string);
    }

As you can see, the comments have been lost and some variable names have changed, but the code is essentially the same and is perfectly readable. And this is with an internal form which has not been designed with decompilation in mind. If decompilation had been designed for in the first place, the binary could have contained pointers to the variable names and comments. Historically we haven't done this, both for 'intellectual property' reasons and because of store poverty. In future, we can and will.

Again, like so much in software, this isn't actually new. The microcomputer BASICs of the seventies and eighties 'tokenised' the source input by the user. This tokenisation was not of course compilation, but it was analogous to it. The internal form of the program that was stored was much terser then the representation the user typed. But when the user asked to list the program, it was expanded into its original form.

Compilation - even compilation into the language of a virtual machine - is much more sophisticated than tokenising, of course. Optimisation means that many source constructs may map onto one object construct, and even that one source construct may in different circumstances map onto many object constructs. Nevertheless it is not impossible - nor even hugely difficult - to decompile object code back into readable, understandable and editable source.

But Java syntax is merely a format. When I type a date into a computer, say '05-02-2005', and ask it to reflect that date back to me, I expect it to be able to reflect back to me '05-02-2006'. But I expect it to be able to reflect back to an American '02-05-2006', and to either of us 'Sunday 5th February 2006' as well. I don't expect the input format to dictate the output format. I expect the output format to reflect the needs and expectations of the person to whom it is displayed.

To summarise, again.

Code is data. The internal representation of data is Don't Know, Don't Care. The output format of data is not constrained by the input format; it should suit the use to which it is to be put, the person to whom it is to be displayed.

Thus if the person to whom my Java code is reflected back is a LISP programmer, it should be reflected back in idiomatic LISP syntax; if a Python programmer, in idiomatic Python syntax. Let us not, for goodness sake, get hung up about syntax; syntax is frosting on the top. What's important is that the programmer editing the code should edit something which is clearly understandable to him or her.

This has, of course, a corollary. In InterLISP, one didn't edit files 'out of core' with a text editor. One edited the source code of functions as S-expressions, in core, with a structure editor. The canonical form of the function was therefore the S-expression structure, and not the printed representation of it. If a piece of code - a piece of executable binary, or rather, of executable DKDC - can be reflected back to users with a variety of different syntactic frostings, none of these can be canonical. The canonical form of the code, which must be stored in version control systems or their equivalent, is the DKDC itself; and to that extent we do care and do need to know, at least to the extent that we need to know that the surface frosting can again be applied systematically to the recovered content of the archive.

# If God does not write LISP

I started my professional life writing LISP on Xerox 1108s and, later, 1186s - Dandelions and Daybreaks, if you prefer names to numbers. When I wanted to multiply two numbers, I multiplied two numbers. I didn't make sure that the result wouldn't overflow some arbitrary store size first. When a function I wrote broke, I edited in its structure in its position on the stack, and continued the computation. I didn't abort the computation, find a source file (source file? How crude and primitive), load it into a text editor, edit the text, save it, check for syntax errors, compile it, load the new binary, and restart the computation. That was more than twenty years ago. It is truly remarkable how software development environments have failed to advance - have actually gone backwards - in that time.

LISP's problem is that it dared to try to behave as though it were a post-scarcity language too soon. The big LISP machines - not just the Xerox machines, the LMI, Symbolics, Ti Explorer machines - were vastly too expensive. My Daybreak had 8Mb of core and 80Mb of disk when PCs usually didn't even have the full 640Kb. They were out-competed by UNIX boxes from Sun and Apollo, which delivered less good software development environments but at a much lower cost. They paid the price for coming too early: they died. And programmers have been paying the price for their failure ever since.

But you only have to look at a fern moss, a frond of bracken, an elm sapling, the water curling over the lip of a waterfall, to know that if God does not write LISP She writes some language so similar to LISP as to make no difference. DNA encodes recursive functions; turbulent fluids move in patterns formed by recursion, whorls within whorls within whorls.

The internal structure, then, of the post scarcity language is rather lisp-like. Don't get hung up on that! Remember that syntax isn't language, that the syntax you see need not be the syntax I see. What I mean by saying the language is lisp-like is that its fundamental operation is recursion, that things can easily be arranged into arbitrary structures, that new types of structure can be created on the fly, that new code (code is just data, after all) can be created and executed on the fly, that there is no primacy of the structures and the code created by the programmer over the structures and code created by the running system; that new code can be loaded and linked seamlessly into a running system at any time. That instead of little discrete programs doing little discrete specialised things in separate data spaces each with its own special internal format and internal structures, the whole data space of all the data available to the machine (including, of course, all the code owned by the machine) exists in a single, complex, messy, powerful pool. That a process doesn't have to make a special arrangement, use a special protocol, to talk to another process or to exchange data with it.

In that pool, the internal storage representation of data objects is DKDC. We neither have nor need to have access to it. It may well change over time without application layer programs even being aware or needing to be aware of the change, certainly without them being recompiled.

The things we can store in the dataspace include:

1. **integers** of any size
1. **reals** to any appropriate degree of precision
1. **rationals, complex numbers**, and other things we might want to compute with
1. **dates, times**, and other such useful things
1. **things people say** of any extent, from names to novels
1. **lists of any extent**, branching or not
1. **slots** associations of names with some setter and, perhaps, getter knowledge which determine what values can be stored under that name
1. **namespaces** collections, extensible or not, of slots
1. **regularities** collections of namespaces each of which share identical names
1. **homogeneities** collections of namespaces each of which share identical slots
1. **functions** all executable things are 'functions' in a lispy sense. They are applied to arguments and return values. They may or may not have internal expectations as to the value type of those arguments.
1. **processes** I don't yet have a good feeling for what a post-scarcity process looks like, at top level. It may simply be a thread executing a function; I don't know. I don't know whether there needs to be one specially privileged executive process.

Things which we no longer store - which we no longer store because they no longer have any utility - include

1. **shorts, longs, doubles**, etc specific internal representation types. You saw that coming.
1. **tables**, and with them, **relational databases** and **relational database management systems** no longer needed because the pool is itself persistent (although achieving the efficiency of data access that mature RDBMS give us may be a challenge).
1. **files** You didn't see that coming?

Files are the most stupid, arbitrary way to store data. Again, with a persistent data pool, they cease to have any purpose. Post scarcity, there are no files and there is no filesystem. There's no distinction between in core and out of core. Or rather, if there are files and a filesystem, if there is a distinction between in core and out of core, that distinction falls under the doctrine of DKDC: we don't know about it, and we don't care about it. When something in the pool wants to use or refer to another something, then that other something is available in the pool. Whether it was there all along, or whether it was suddenly brought in from somewhere outside by the runtime system, we neither know nor care. If things in the pool which haven't been looked at for a long time are sent to sulk elsewhere by the runtime system that is equally uninteresting. Things which are not referenced at all, of course, may be quietly dropped by the runtime system in the course of normal garbage collection.

One of the things we've overloaded onto the filesystem is security. In core, in modern systems, each process guards its own pool of store jealously, allowing other processes to share data with it only through special channels and protocols, even if the two processes are run by the same user identity with the same privilege. That's ridiculous. Out of core, data is stored in files often with inscrutable internal format, each with its own permissions and access control list.

It doesn't need to be that way. Each primitive data item in core - each integer, each list node, each slot, each namespace - can have its own access control mechanism. Processes, as such, will never 'own' data items, and will certainly never 'own' chunks of store - at the application layer, even the concept of a chunk of store will be invisible. A process can share a data item it has just created simply by setting an appropriate access policy on it, and programmers will be encouraged normally to be as liberal in this sharing as security allows. So the slot Salary of the namespace Simon might be visible only to the user Simon and the role Payroll, but that wouldn't stop anyone else looking at the slot Phone number of the same namespace.

Welcome, then, to post scarcity computing. It may not look much like what you're used to, but if it doesn't it's because you've grown up with scarcity, and even since we left scarcity behind you've been living with software designed by people who grew up with scarcity, who still hoard when there's no need, who don't understand how to use wealth. It's a richer world, a world without arbitrary restrictions. If it looks a lot like Alan Kay (and friends)'s Croquet, that's because Alan Kay has been going down the right path for a long time.