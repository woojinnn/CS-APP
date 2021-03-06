# KAIST CS230: System Programming

## Logistics

- Instructor: [Jeehoon Kang](https://cp.kaist.ac.kr/jeehoon.kang)
- TA: [Kyeongmin Cho](https://cp.kaist.ac.kr/kyeongmin.cho) (head), [Sungsoo Han](https://cp.kaist.ac.kr/sungsoo.han), [Sunghwan Shim](https://cp.kaist.ac.kr/sunghwan.shim), [Jaemin Choi](https://cp.kaist.ac.kr/jaemin.choi), [Jaehwang Jung](https://cp.kaist.ac.kr/jaehwang.jung), Chanyoung Chung
  See [below](#communication) for communication policy.
    + Office Hour: TBA. See [below](#communication) for office hour policy.
- Time & Place: Mon & Wed 10:30am-11:45am, ~~Terman Hall, Bldg. E11~~ Online (announced in [chat](https://cp-cs230.kaist.ac.kr/#narrow/stream/3-cs230-announcement))
- Course Website: https://cp-git.kaist.ac.kr/cs230/cs230
- Chat Website: https://cp-cs230.kaist.ac.kr
- Lab Submission website: https://gg.kaist.ac.kr/course/7
- Announcements: in [issue tracker](https://cp-git.kaist.ac.kr/cs230/cs230/-/issues?label_name%5B%5D=announcement) or in [chat](https://cp-cs230.kaist.ac.kr/#narrow/stream/3-cs230-announcement)
    + We assume you read each announcement within 24 hours.
    + We strongly recommend you to watch the repository and use desktop/mobile Zulip client for prompt access to the announcements.


### Online sessions

Due to COVID-19, we're going to conduct online sessions for this semester.
Please see the [announcement](https://cp-cs230.kaist.ac.kr/#narrow/stream/3-cs230-announcement) for more details.



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


### Prerequisites

- It is **required** that students already took courses on:

    + Basic understanding of programming (CS101)
    + Basic understanding of mathematics (MAS101)

  Without a proper understanding of these topics, you will highly likely struggle in this course.



## Grading & honor code

### Honor code

[Please sign KAIST School of Computing Honor Code here](https://gg.kaist.ac.kr/quiz/35/).

#### Cheating

**IMPORTANT: PAY CLOSE ATTENTION. VERY SERIOUS.**

- Cheating is including, but not limited to, the following activities:

    + *Sharing*: code, document, or any products by copying, retyping, **looking at**, or supplying a file​
    + *Describing*: verbal description of code from one person to another
    + *Coaching*: helping your friend to write a lab, line by line​
    + *Searching*: **the Web for solutions​**
    + *Copying*: code from a previous course or online solution​ (you are only allowed to use code we supply)

- Cheating doesn't include the following activities:

    + Explaining how to use systems or tools​
    + Helping others with high-level design issues

- **Cheating will be harshly punished.**

    + I will raise an issue to the Reward and Punishment Committee.
    + Ignorance is no excuse.
    + So don't do it and start early.

- We will use sophisticated tools for detecting code plagiarism​.

    + [Google "code plagiarism detector" for images](https://www.google.com/search?q=code+plagiarism+detector&tbm=isch) and see how these tools can detect "sophisticated" plagiarisms.
      You really cannot escape my catch. Just don't try plagiarism in any form.


### Programming assignments (40% with different weights among labs)

There will be about 6 or 7 programming assignments. The assignments are the most important part of
this course. All the assignments are single-student assignments.

We will check for cheating (including copying). You are not allowed to use others' code unless otherwise specified in the lab README.

Assignments will be due at 11:59pm on the specified due date. **After the due date, your submission will not be accepted.**


#### Evaluation

All submitted C files should be properly formatted. Otherwise, you will get 0 points.
Before submission, format your file with `make format`:

```
$ make format
```

Your solution will be tested on a same Linux machine that you were provided with.
Some labs are machine-dependent, so do work on your provided environment.

You are free to modify other files, but make sure your solution works with the original files.

#### Hand-in Instructions

Go to [the submission website](https://gg.kaist.ac.kr/course/7/) and submit solution file(s).

Choose the corresponding lab in "ASSIGNMENTS" table; upload your solution file(s); and then click the submit button.

"SUCCESS?" just shows whether your solution runs without any errors. It does not necessarily mean that you get a perfect score.
**So please check the log and see if the result is as expected.**

#### Tools

Make sure you're capable of using the following development tools:

- [Git](https://git-scm.com/): for downloading the homework skeleton and version-controlling your
  development. If you're not familiar with Git, walk through [this
  tutorial](https://www.atlassian.com/git/tutorials).

    + **IMPORTANT**: you should not expose your work to others.
      For this purpose, you will be provided with a [Gitlab](https://cp-git.kaist.ac.kr) account (see [below](#communication)).

    + Fork this repository to your private namespace in <https://cp-git.kaist.ac.kr>; make it private; and then clone it.
        * Forking a project: https://docs.gitlab.com/ee/user/project/repository/forking_workflow.html#creating-a-fork
        * Making the project private: https://docs.gitlab.com/ee/public_access/public_access.html#how-to-change-project-visibility
        * Cloning the project: https://docs.gitlab.com/ee/gitlab-basics/start-using-git.html

    + To get updates from the upstream, fetch and merge `upstream/main`.

      ```bash
      $ git remote add upstream ssh://git@cp-git.kaist.ac.kr:9001/cs230/cs230.git
      $ git remote -v
      origin	 ssh://git@cp-git.kaist.ac.kr:9001/<your-id>/cs230.git (fetch)
      origin	 ssh://git@cp-git.kaist.ac.kr:9001/<your-id>/cs230.git (push)
      upstream ssh://git@cp-git.kaist.ac.kr:9001/cs230/cs230.git (fetch)
      upstream ssh://git@cp-git.kaist.ac.kr:9001/cs230/cs230.git (push)
      $ git fetch upstream
      $ git merge upstream/main
      ```

    + You may push your local development to the Gitlab server.

      ```bash
      $ git push -u origin main
      ```

- [Visual Studio Code](https://code.visualstudio.com/) (optional): for developing your homework. If you prefer other editors, you're good to go.


- You can connect to server by `ssh s<student-id>@cp-service.kaist.ac.kr -p13000`, e.g., `ssh s20071163@cp-service.kaist.ac.kr -p13000`.

    + **IMPORTANT: Don't try to hack. Don't try to freeze the server. Please be nice.**

    + Your initial password is `123454321`. IMPORTANT: you should change it ASAP.

    + I require you to register public SSH keys to the server. (In March, we'll expire your password so that you can log in only via SSH keys.)
      See [this tutorial](https://serverpilot.io/docs/how-to-use-ssh-public-key-authentication/) for more information on SSH public key authentication.
      Use `ed25519`.

    + In your client, you may want to set your `~/.ssh/config` as follows for easier SSH access:

      ```
      Host cs230
      Hostname cp-service.kaist.ac.kr
      Port 13000
      User s20071163
      ```

      Then you can connect to the server by `ssh cs230`.

    + Now you can [use it as a VSCode remote server as in the video](https://www.youtube.com/watch?v=TTVuUIhdn_g&list=PL5aMzERQ_OZ8RWqn-XiZLXm1IJuaQbXp0&index=3).



### Midterm and final exams (30% + 30%)

- Study Guide: at least 2/3 of points will be related to lab assignments.

- Date & Time: April 21st (midterm) and June 16th (final), 09:00am-11:45am (or shorter, TBA)

- Place: online

    + You need to set up a separate camera that shows you, your hand, pencil and paper, and monitor, as in [this picture](https://user-images.githubusercontent.com/1201316/95432855-28d33800-098a-11eb-9b18-b515c34bb2e9.jpg).
      If you cannot do so, you will not be able to take this course.

- Appealing: TBA


### Attendance (?%)

- You should solve a quiz at the [Course Management](https://gg.kaist.ac.kr/course/7) website for each session. **You should answer to the quiz by the end of the day.**

- If you miss a significant number of sessions, you'll automatically get an F.



## Communication

### Registration

- Make sure you can log in the [chat website](https://cp-cs230.kaist.ac.kr).
  If you cannot, please see KLMS/email announcements first, and then send an email to the head TA.

- Retrieve Zoom session information at chat's [announcement stream](https://cp-cs230.kaist.ac.kr/#narrow/stream/3-cs230-announcement).

- Make sure you can log in [Gitlab](https://cp-git.kaist.ac.kr).

    + An email should have been sent to your `@kaist.ac.kr` email address for confirming the address. Please do so.
      If not, please contact the head TA in the chat.

    + Reset your password by clicking "Forgot your password?" [here](https://cp-git.kaist.ac.kr/users/sign_in).

    + Log in with your email address and the new password.
      If you cannot, please contact the head TA in the chat.

    + Assignments will be hosted here: https://cp-git.kaist.ac.kr/cs230/cs230

- Make sure you can log in the [lab submission website](https://gg.kaist.ac.kr).

    + Reset your password here: https://gg.kaist.ac.kr/accounts/password_reset/
      The email address is your `@kaist.ac.kr` address.

    + The id is your student id (e.g., 20071163).

    + Log in with your email address and the new password.
      If you cannot, please contact the head TA in the chat.

    + Sign [the honor code](https://gg.kaist.ac.kr/quiz/35/) by March 12nd.
      Otherwise, you will be expelled from the class.


### Rules

- Course-related announcements and information will be posted on the
  [website](https://cp-git.kaist.ac.kr/cs230/cs230) or on the
  [Zulip announcement](https://cp-cs230.kaist.ac.kr/#narrow/stream/3-cs230-announcement).
  You are expected to read all announcements within 24 hours of their being posted. It is highly
  recommended to watch the repository so that new announcements will automatically be delivered to
  you email address.

- Ask questions on lab assignments in the dedicated streams ([example](https://cp-cs230.kaist.ac.kr/#narrow/stream/5-cs230-lab-linkedlistlab)).
    + Don't DM to the instructor or TAs.
    + Before asking a question, search for similar ones in Zulip and Google.
    + Describe your question as detailed as possible. It should include following things:
      * Environment (OS, gcc, g++ version, and any other related program information). You can just say provided server if you use it without any changes.
      * Command(s) that you used and the result. Any logs should be formatted in code. Refer to [this](https://zulip.com/help/format-your-message-using-markdown#code).
      * Any directory or file changes you've made. If it is solution file, just describe which part of the code is modified.
      * Googling result. Search before asking, and share the keyword used for searching and what you've learned from it.
    + Properly Name your question as the topic.
    + Read [this](https://cp-git.kaist.ac.kr/cs230/cs230#communication) for more instructions.

- Ask questions on lectures in the dedicated stream (e.g., https://cp-cs230.kaist.ac.kr/#narrow/stream/4-cs230-lecture).
    + The instructions for lab assignment questions apply also to lecture questions.

- Ask your questions via [Zulip](https://cp-cs230.kaist.ac.kr/) private messages to
  [the instructor](https://cp.kaist.ac.kr/jeehoon.kang) or [the head TA](https://cp.kaist.ac.kr/kyeongmin.cho)
  **only if** they are either confidential or personal. Otherwise, ask questions in the
  appropriate stream. Unless otherwise specified, don't send emails to the instructor or TAs.
  Any questions failing to do so (e.g. email questions on course materials) will not be answered.

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

- Your Zoom name should be `<your student number> <your name>` (e.g., 20071163 강지훈).
  Change your name by referring to [this](https://support.zoom.us/hc/en-us/articles/201363203-Customizing-your-profile).

- This course is conducted in English. But you may ask questions in Korean. Then I will translate it to English.
