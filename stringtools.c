/**
 * Shared from Viking Mud core library, as it is referenced from Clippy.
 */

/*
** FUNCTION
**     int strsrch(string str, string substr, varargs int flag)
**
** DESCRIPTION
**     searches for the first occurance of the string 'substr'
**     in the string 'str'. The last occurance of 'substr' can
**     be found by passing -1 as the 3rd argument (optional).
**
**     the integer offset of the first (last) match is returned.
**     if there was no match, -1 is returned.
*/

int strsrch(string str, string substr, varargs int flag) {
    int i, x;

    if (!stringp(str) || (!stringp(substr)) || !strlen(str) || !strlen(substr))
        return -1; 
  
    if (flag == -1) {

        i = strlen(str)-1;

        while (i >= 0) {
            x = i - (strlen(substr) - 1);
            if (x < 0)
                x = 0;
            if (str[x .. i] == substr)
                return x;
            else
                i--;
        }
    }

    else {

        i = 0;

        while (i <= strlen(str) - 1) {
            x = i + (strlen(substr) - 1);
            if (x > strlen(str) - 1)
                x = strlen(str) - 1;
            if (str[i .. x] == substr)
                return i;
            else
                i++;
        }
    }

    return -1;
}
