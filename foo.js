function print(auto m) { asm print m; }

function isAnswerCorrect(int answer) -> auto {
    var bool result;
    var int correct_answer = 42;
    asm ieq result correct_answer answer;
    return result;
}

function main() -> int {
    var int answer = 42;
    var bool correct;
    correct = isAnswerCorrect(answer);

    if correct {
        var string s = "answer OK";
        print(s);
    }

    return 0;
}

