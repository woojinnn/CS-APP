# KAIST CS230: System Programming

## Logistics

- Instructor: [Jeehoon Kang](https://cp.kaist.ac.kr/jeehoon.kang)
- TA: [Kyeongmin Cho](https://cp.kaist.ac.kr/kyeongmin.cho) (head), [Sungsoo Han](https://cp.kaist.ac.kr/sungsoo.han), [Sunghwan Shim](https://cp.kaist.ac.kr/sunghwan.shim). See [below](#communication) for communication policy.
    + Office Hour: TBA. See [below](#communication) for office hour policy.
- Time & Place: Mon & Wed 10:30am-11:45am, ~~Terman Hall, Bldg. E11~~ Online (TBA)
- Website: https://cp-git.kaist.ac.kr/cs230/cs230
- Announcements: in [issue
  tracker](https://cp-git.kaist.ac.kr/cs230/cs230/-/issues?label_name%5B%5D=announcement)



### Online sessions

Due to COVID-19, we're going to conduct online sessions for this semester.
The details are TBA.



## Course description

### Context 1: about abstractions

> All problems in computer science can be solved by another level of indirection [abstraction layer]---except for the problem of too many layers of indirection.
>
> --- David Wheeler

Computer science is fundamentally about solving real-world problems with [**abstraction**](https://www.youtube.com/watch?v=x4B0CCrdVoU&feature=youtu.be).
As an undergraduate students, you are going to learn historically crucial ideas on abstraction that actually solved such problems and achieved tremendous commercial success.
(As a side-benefit, You may be able to **build** such ideas :)

An abstraction needs to serve two purposes.
First, it needs to be **simple**. The primary purpose of an abstraction is let the users to forget about unnecessary details in a particular context.
Second, it needs to be **relevant** to the real-world. A non-relevant abstraction, however simple it is, does not help the users to understand and work on real-world problem.
Thus building an abstraction is an art of striking the balance between the two criteria. For these reasons, multiple abstractions often compete with each other with different trade-offs.

In [CS101](http://cs101.kaist.ac.kr/), you may have learned historically crucial abstractions of computing in general.
In CS230 (this course), we will have a more in-depth focus on historically crucial abstractions of physical computer systems (or in other words, machines).
Put differently, this course is an introduction to realization of computing in physical worlds.



### Context 2: about computer systems

Computer systems drive the key innovations in the field of computing.
For an example of the importance of computer systems, let's think of GPS navigation applications.
You can input your desire to reach a destination, either via touchpad or via microphone, and then the application will show you an optimal route to the destination.
This process involves mobilization of (physical) computer systems, including but not limited to:

- Processing signals from input devices
- Understanding the user's intent
- Finding out the optimal route
- Presenting the route to output devices
- Continuously monitoring road conditions (probably from online) and adapting the route accordingly

Without proper utilization of computer systems, not only GPS applications but also applications of deep learning, IoT, human-computer interaction, etc. cannot be made possible.

To use a GPS application (and all the other applications), you don't need to know the system's every detail thanks to abstraction: all you need to know is how to interact (input/output) with the application.
To some degree, it's true also to implement such systems: for example, if you're in charge of input devices, you don't need to know every detail of output devices.
And yet you probably should know a general, high-level understanding of computer system structuring principles in order to communicate with colleagues.
For example, you at least should understand how input/output typically work among multiple components of computer systems to build proper input devices.



### Goal: learning abstractions of computer systems

CS230 is a *crash course* (or rather, a *whirlwind tour*) for learning how computer systems work in general.
More specifically, you're going to learn crucial abstractions for computer systems that are historically proven to be simple (from programmer's perspective) and relevant (from builder's perspective) at the same time.

This course (which teaches computational abstraction of systems) complements with [CS220: Programming Principles](https://softsec.kaist.ac.kr/courses/2020s-cs220/) (which teaches computational abstraction of mathematics).
This course serves as a foundation for later courses on computer organization, operating systems, compilers, and networks.



### Textbook

- Randal E. Bryant and David R. O'Hallaron, [Computer Systems: A Programmer's Perspective, Third Edition](https://csapp.cs.cmu.edu/), Prentice Hall, 2011
    + Video lectures by the authors available [here](https://scs.hosted.panopto.com/Panopto/Pages/Sessions/List.aspx#folderID=%22b96d90ae-9871-4fae-91e2-b1627b43e25e%22&sortColumn=1&sortAscending=true)
- Brian W. Kernighan and Dennis M. Ritchie, The C Programming Language, Second Edition, Prentice Hall, 1988 (optional)
- [The Missing Semester of Your CS Education](https://missing.csail.mit.edu/) (optional)
    + [Korean translation](https://missing-semester-kr.github.io/)

### Tools

Make sure you're capable of using the following development tools:

- [Git](https://git-scm.com/): for downloading the homework skeleton and version-controlling your
  development. If you're not familiar with Git, walk through [this
  tutorial](https://www.atlassian.com/git/tutorials).

    + **IMPORTANT**: you should not expose your work to others. In particular, you should not fork
      the [upstream](https://cp-git.kaist.ac.kr/cs230/cs230) and push there. Please the following
      steps:

        * Directly clone the upstream without forking it.

          ```bash
          $ git clone --origin upstream https://cp-git.kaist.ac.kr/cs230/cs230.git
          $ cd cs230
          $ git remote -v
          upstream	https://cp-git.kaist.ac.kr/cs230/cs230.git (fetch)
          upstream	https://cp-git.kaist.ac.kr/cs230/cs230.git (push)
          ```

        * To get updates from the upstream, fetch and merge `upstream/main`.

          ```bash
          $ git fetch upstream
          $ git merge upstream/main
          ```

    + If you want to manage your development in a Git server, please create your own private
      repository.

        * You may use a cp-git.kaist.ac.kr account we will provide.

        * You may also upgrade your GitHub account to "PRO", which is free of charge. Refer to the
          [documentation](https://education.github.com/students).

        * Set up your repository as a remote.

          ```bash
          $ git remote add origin ssh://git@cp-git.kaist.ac.kr:9001/<your-id>/cs230.git
          $ git remote -v
          origin	 ssh://git@cp-git.kaist.ac.kr:9001/<your-id>/cs230.git (fetch)
          origin	 ssh://git@cp-git.kaist.ac.kr:9001/<your-id>/cs230.git (push)
          upstream https://cp-git.kaist.ac.kr/cs230/cs230.git (fetch)
          upstream https://cp-git.kaist.ac.kr/cs230/cs230.git (push)
          ```

          Alternatively, you may use a GitHub private repository as follows.

          ```bash
          $ git remote add origin git@github.com:<github-id>/cs230.git
          ...
          ```

        * Push to your repository.

          ```bash
          $ git push -u origin main
          ```

- [Visual Studio Code](https://code.visualstudio.com/) (optional): for developing your homework. If
  you prefer other editors, you're good to go.
      
- You can connect to server by `ssh s<student-id>@cp-service.kaist.ac.kr -p13000`, e.g., `ssh
  s20071163@cp-service.kaist.ac.kr -p13000`. See [this
  issue](https://cp-git.kaist.ac.kr/cs230/cs230/-/issues/2) for more detail.

    + Add the following lines in your `~/.ssh/config`:
    
      ```
      Host cs230
        Hostname cp-service.kaist.ac.kr
        Port 14000
        User s<student-id>
      ```
      
      Then you can connect to the server by `ssh cs230`.

    + Now you can [use it as a VSCode remote server as in the video](https://www.youtube.com/watch?v=TTVuUIhdn_g&list=PL5aMzERQ_OZ8RWqn-XiZLXm1IJuaQbXp0&index=3).



## Prerequisites

- It is **required** that students already took courses on:

    + Basic understanding of programming (CS101)
    + Basic understanding of mathematics (MAS101)

  Without a proper understanding of these topics, you will highly likely struggle in this course.



## Grading & honor code

### Programming assignments (30%)

There will be about 6 or 7 programming assignments. The assignments are the most important part of
this course. All the assignments are single-student assignments.

Assignments will be due at 11:59pm on the specified due date. **After the due date, your submission
will not be accepted.**


### Midterm and final exams (70%)

The exams will evaluate your theoretical understanding of shared mutable states.


### Attendance (?%)

You should solve a quiz at the [Course Management](https://gg.kaist.ac.kr/course/7) website for each
session. **You should answer to the quiz by the end of the day.**


### Honor code

[Please sign KAIST School of Computing Honor Code here](https://gg.kaist.ac.kr/quiz/35/).



## Communication

- Course-related announcements and information will be posted on the
  [website](https://cp-git.kaist.ac.kr/cs230/cs230) or on the [issue
  tracker](https://cp-git.kaist.ac.kr/cs230/cs230/-/issues). You are expected to read all
  announcements within 24 hours of their being posted. It is highly recommended to watch the
  repository so that new announcements will automatically be delivered to you email address.

- Ask your questions via email to [the instructor](https://cp.kaist.ac.kr/jeehoon.kang) or [the head
   TA](https://cp.kaist.ac.kr/kyeongmin.cho) **only if** they are either confidential or
   personal. Otherwise, ask questions in the [issue
   tracker](https://cp-git.kaist.ac.kr/cs230/cs230/-/issues). Unless otherwise specified, don't send
   emails to non-head TAs. Any questions failing to do so (e.g. email questions on course materials)
   will not be answered.

    + I'm requiring you to ask questions online first for two reasons. First, clearly writing a
      question is the first step to reach an answer. Second, you can benefit from questions and
      answers of other students.

- We are NOT going to discuss *new* questions during the office hour. Before coming to the office
  hour, please check if there is a similar question on the issue tracker. If there isn't, file a new
  issue and start discussion there. The agenda of the office hour will be the issues that are not
  resolved yet.

- Emails to the instructor or the head TA should begin with "CS230:" in the subject line, followed
  by a brief description of the purpose of your email. The content should at least contain your name
  and student number. Any emails failing to do so (e.g. emails without student number) will not be
  answered.
