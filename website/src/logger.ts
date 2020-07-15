import process from 'process';
import chalk from 'chalk';

export enum LoggerLevel {
  DEBUG,
  INFO,
  WARN,
  ERROR,
  FATAL
};

export class Logger {
  l_Prefix: string;
  l_Level: LoggerLevel;

  public constructor(l_Prefix: string, l_Level?: LoggerLevel) {
    if (!l_Level) l_Level = LoggerLevel.INFO;

    this.l_Level = l_Level;
    this.l_Prefix = l_Prefix;
  }

  public error(e: Error)
  {
    this.lprint(e.message, LoggerLevel.ERROR);
  }

  public lprint(raw: string, tempLevel: LoggerLevel)
  {
    // Stores the old level, sets the new one, prints
    // - and restores the original one
    const old: LoggerLevel = this.l_Level;
    this.l_Level = tempLevel;
    this.print(raw);
    this.l_Level = old;
  }

  public print(raw: string) {
    let message: string = "";

    // Adds the thread id to the message and
    // - adds the logger level to the final message
    message += `P${process.pid}->`;
    switch (this.l_Level) {
      case LoggerLevel.DEBUG: {
        message += chalk.blueBright(`[DEBUG@${this.l_Prefix}]: `);
        break;
      }
      case LoggerLevel.ERROR: {
        message += chalk.redBright(`[ERROR@${this.l_Prefix}]: `);
        break;
      }
      case LoggerLevel.FATAL: {
        message += chalk.red(`[FATAL@${this.l_Prefix}]: `);
        break;
      }
      case LoggerLevel.INFO: {
        message += chalk.green(`[INFO@${this.l_Prefix}]: `);
        break;
      }
      case LoggerLevel.WARN: {
        message += chalk.yellow(`[WARN@${this.l_Prefix}]: `);
        break;
      }
    }

    // Adds the raw message itself to the final message, and prints
    // - it to the console
    message += raw;
    console.log(message);
  }
}