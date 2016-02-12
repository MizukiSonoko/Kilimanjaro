; ModuleID = 'Sharo'

@0 = private unnamed_addr constant [13 x i8] c"Rize senpai\0A\00"

define void @main() {
entrypoint:
  %0 = call i32 @puts(i8* getelementptr inbounds ([13 x i8]* @0, i32 0, i32 0))
  ret void
}

declare i32 @puts(i8*)